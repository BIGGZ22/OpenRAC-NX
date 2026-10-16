//! VU0 in **micro mode**: a program in VU0's own 4 KB of micro memory, started from the EE with
//! `vcallms` and run to its end bit.
//!
//! Each 64-bit instruction is an *upper* half (the FMAC: add/sub/mul/madd… on four lanes, through
//! [`ps2_float::vu`]) and a *lower* half (loads and stores, integer registers and branches, the divider, flag
//! reads) — or, with the `I` bit set, an upper half and a 32-bit constant for the `I` register.
//!
//! # Timing
//!
//! A micro program's results can depend on its schedule, so the pipelines are modelled the way the reference
//! emulator (PCSX2's microVU) models them:
//!
//! * **FMAC results take 4 cycles.** A read of a lane that is not ready yet stalls until it is; the result
//!   is the same as running in order, but the stall moves everything after it.
//! * **`Q` lands 7 cycles after `div`/`sqrt` issue, 13 after `rsqrt`.** An instruction before that reads the
//!   *old* `Q` (no stall) — which is what lets a program overlap a divide with other work. `waitq`, and a
//!   second divide while one is in flight, stall until it lands.
//! * **Flags land 4 cycles after the instruction that set them.** `fsand`, `fmand`, `fcand`… read the MAC,
//!   status and clipping flags as they were left by the FMAC instructions issued at least 4 cycles earlier.
//! * Upper and lower halves read their operands before either writes; the `I` an upper half reads is the
//!   previous one (the constant in the same instruction lands after it); a branch runs its delay slot; the
//!   instruction after the one with the `E` bit runs too, then the pipelines drain and the program is over.
//!
//! Not modelled: the overflow and underflow flags (`O`/`U`, as in microVU only `Z` and `S` are computed: `Z`
//! for a zero lane, `S` for a set sign bit, so `-0` sets both); the hazard where a branch reads an integer
//! register the instruction just before it wrote (integer results are available to the next instruction
//! here); the `M`, `D` and `T` bits; the `R` register; and VU1-only operations (`xgkick`, `xtop`, `mfp`, the
//! EFU), which panic by name.
//!
//! The EE side ([`crate::Ee`]'s `vcallms`) runs a program to completion at once, so the VU0 registers the
//! EE reads after it hold the final values; each run starts with the pipelines empty.

// lane loops index the dest mask, the result and the register file by the same lane number
#![allow(clippy::needless_range_loop)]

use crate::Ee;
use ps2_float::{fpu, vu};
use std::collections::VecDeque;

/// Size of VU0's micro memory and of its data memory, in bytes.
pub const MEM: usize = 0x1000;

const FMAC: u64 = 4;
const FLAGS: u64 = 4;
const DIV: u64 = 7;
const RSQRT: u64 = 13;

/// The pipelines of one run: when each register lane is ready, and the results still on their way.
struct Pipe {
    cycle: u64,
    ready: [[u64; 4]; 32],
    /// `Q` (value, cycle it lands, the divider's `I`/`D` status bits).
    q: Option<(f32, u64, u32)>,
    /// MAC and status flags (cycle they land, MAC, status Z/S bits).
    flags: VecDeque<(u64, u32, u32)>,
    /// Clipping judgements (cycle they land, the six new bits).
    clip: VecDeque<(u64, u32)>,
}

/// Lanes of a `dest` field (bit 3 = x … bit 0 = w).
fn has(dest: u32, k: usize) -> bool {
    dest >> (3 - k) & 1 == 1
}

fn dest_lanes(dest: u32) -> [bool; 4] {
    std::array::from_fn(|k| has(dest, k))
}

fn one_lane(k: usize) -> [bool; 4] {
    std::array::from_fn(|i| i == k)
}

/// Where an upper half's result goes.
enum Out {
    Vf(usize),
    Acc,
}

/// What an upper half did.
enum Up {
    Nothing,
    /// (target, dest, the four lanes, whether it sets the MAC/status flags)
    Write(Out, u32, [f32; 4], bool),
    /// `clip`: the six new judgement bits
    Clip(u32),
}

impl Ee {
    /// Run the micro program at `start` (in 8-byte units, as `vcallms` gives it) until its end bit, or panic
    /// after `budget` instructions. Leaves the VU0 registers and flags as the program left them, and its
    /// cycle count in [`Ee::vu0_cycles`].
    pub fn vu0_run(&mut self, start: u32, budget: u64) {
        let mut p = Pipe { cycle: 0, ready: [[0; 4]; 32], q: None, flags: VecDeque::new(), clip: VecDeque::new() };
        let slots = (MEM / 8) as u32;
        let (mut pc, mut npc) = (start % slots, (start + 1) % slots);
        let mut ending = false;
        for _ in 0..budget {
            let at = 8 * pc as usize;
            let word = |k: usize| u32::from_le_bytes(self.vu0_micro[at + k..at + k + 4].try_into().unwrap());
            let (lo, up) = (word(0), word(4));
            let here = pc;
            pc = npc;
            npc = (pc + 1) % slots;
            let loi = up >> 31 & 1 == 1;
            if let Some(t) = self.vu0_step(&mut p, up, if loi { None } else { Some(lo) }, here) {
                npc = t % slots;
            }
            if loi {
                self.i = f32::from_bits(lo);
            }
            if ending {
                self.vu0_drain(&mut p);
                self.vu0_cycles = p.cycle;
                return;
            }
            ending = up >> 30 & 1 == 1;
        }
        panic!("VU0 micro program at {start:#x} (8-byte units): no end bit after {budget} instructions (pc {pc:#x})");
    }

    /// Land everything still in flight, as the end of a program does.
    fn vu0_drain(&mut self, p: &mut Pipe) {
        let ready = p.ready.iter().flatten().copied();
        let pending = p.q.map(|q| q.1).into_iter().chain(p.flags.iter().map(|f| f.0)).chain(p.clip.iter().map(|c| c.0));
        let last = ready.chain(pending).fold(p.cycle, u64::max);
        self.vu0_land(p, last);
        p.cycle = last;
    }

    /// Apply the results due by `cycle`: `Q`, then flags and clipping judgements in issue order.
    fn vu0_land(&mut self, p: &mut Pipe, cycle: u64) {
        if let Some((q, at, id)) = p.q
            && at <= cycle
        {
            self.q = q;
            self.status = self.status & !0x30 | id | id << 6;
            p.q = None;
        }
        while let Some(&(at, mac, zs)) = p.flags.front() {
            if at > cycle {
                break;
            }
            self.mac = mac;
            self.status = self.status & !0xf | zs | zs << 6;
            p.flags.pop_front();
        }
        while let Some(&(at, bits)) = p.clip.front() {
            if at > cycle {
                break;
            }
            self.clip = (self.clip << 6 | bits) & 0xff_ffff;
            p.clip.pop_front();
        }
    }

    /// One instruction pair: stall for its operands, land what is due, run both halves. Returns a branch
    /// target (taken after the delay slot).
    fn vu0_step(&mut self, p: &mut Pipe, up: u32, lo: Option<u32>, here: u32) -> Option<u32> {
        let mut issue = p.cycle;
        let mut wait = |reg: usize, lanes: [bool; 4]| {
            for k in 0..4 {
                if lanes[k] && reg != 0 {
                    issue = issue.max(p.ready[reg][k]);
                }
            }
        };
        for (reg, lanes) in self.upper_reads(up) {
            wait(reg, lanes);
        }
        let lower = lo.map(|l| self.lower_reads(l));
        if let Some((reads, _)) = &lower {
            for &(reg, lanes) in reads {
                wait(reg, lanes);
            }
        }
        if let (Some((_, true)), Some(q)) = (&lower, p.q) {
            issue = issue.max(q.1); // waitq, or a divide while the divider is busy
        }
        p.cycle = issue;
        self.vu0_land(p, issue);

        // both halves read the registers as they were before this instruction
        let (vf, vi, acc, i, q) = (self.vf, self.vi, self.vacc, self.i, self.q);
        let upper = self.upper(up, &vf, acc, i, q, here);
        let mut target = None;
        let mut lower_vf: Option<(usize, u32, [f32; 4])> = None;
        if let Some(l) = lo {
            target = self.lower(p, l, &vf, &vi, issue, &mut lower_vf, here);
        }
        if let Some((reg, dest, v)) = lower_vf {
            self.write_vf(p, reg, dest, v, issue);
        }
        if let Up::Clip(bits) = upper {
            p.clip.push_back((issue + FLAGS, bits));
        }
        if let Up::Write(out, dest, v, flags) = upper {
            match out {
                Out::Vf(reg) => self.write_vf(p, reg, dest, v, issue),
                Out::Acc => {
                    for k in 0..4 {
                        if has(dest, k) {
                            self.vacc[k] = v[k];
                        }
                    }
                }
            }
            if flags {
                let mut mac = 0;
                for k in 0..4 {
                    if has(dest, k) {
                        let bit = 1 << (3 - k);
                        if v[k] == 0.0 {
                            mac |= bit;
                        }
                        if v[k].is_sign_negative() {
                            mac |= bit << 4;
                        }
                    }
                }
                let zs = (mac & 0xf != 0) as u32 | ((mac & 0xf0 != 0) as u32) << 1;
                p.flags.push_back((issue + FLAGS, mac, zs));
            }
        }
        p.cycle = issue + 1;
        target
    }

    fn write_vf(&mut self, p: &mut Pipe, reg: usize, dest: u32, v: [f32; 4], issue: u64) {
        if reg == 0 {
            return;
        }
        for k in 0..4 {
            if has(dest, k) {
                self.vf[reg][k] = v[k];
                p.ready[reg][k] = issue + FMAC;
            }
        }
    }

    /// The float register lanes an upper half reads.
    fn upper_reads(&self, up: u32) -> Vec<(usize, [bool; 4])> {
        let (dest, ft, fs, fun) = (up >> 21 & 15, (up >> 16 & 31) as usize, (up >> 11 & 31) as usize, up & 63);
        let d = dest_lanes(dest);
        let xyz = [true, true, true, false];
        if fun < 0x3c {
            return match fun {
                0..=0x1b => vec![(fs, d), (ft, one_lane((fun & 3) as usize))],
                0x1c..=0x27 => vec![(fs, d)],
                0x2e => vec![(fs, xyz), (ft, xyz)],
                _ => vec![(fs, d), (ft, d)],
            };
        }
        match (up >> 6 & 31) << 2 | fun & 3 {
            op @ (0..=0x0f | 0x18..=0x1b) => vec![(fs, d), (ft, one_lane((op & 3) as usize))],
            0x2e => vec![(fs, xyz), (ft, xyz)],
            0x1f => vec![(fs, xyz), (ft, one_lane(3))], // clip
            0x2f => vec![],
            0x28..=0x2d => vec![(fs, d), (ft, d)],
            _ => vec![(fs, d)],
        }
    }

    /// The upper half, computed but not yet written.
    fn upper(&self, up: u32, vf: &[[f32; 4]; 32], acc: [f32; 4], i: f32, q: f32, here: u32) -> Up {
        let (dest, ft, fs, fd, fun) =
            (up >> 21 & 15, (up >> 16 & 31) as usize, (up >> 11 & 31) as usize, (up >> 6 & 31) as usize, up & 63);
        let (a, b) = (vf[fs], vf[ft]);
        let bc = [b[(up & 3) as usize]; 4];
        let lanes = |f: &dyn Fn(usize) -> f32| -> [f32; 4] { std::array::from_fn(f) };
        let (cy, cz) = ([1, 2, 0, 3], [2, 0, 1, 3]); // the outer-product lane pattern
        // the arithmetic shared by the vf-target and accumulator-target forms
        let fmac = |op: u32| -> Option<[f32; 4]> {
            Some(match op {
                0..=3 => lanes(&|k| vu::add(a[k], bc[k])),
                4..=7 => lanes(&|k| vu::sub(a[k], bc[k])),
                8..=11 => lanes(&|k| vu::madd(acc[k], a[k], bc[k])),
                12..=15 => lanes(&|k| vu::msub(acc[k], a[k], bc[k])),
                0x18..=0x1b => lanes(&|k| vu::mul(a[k], bc[k])),
                0x1c => lanes(&|k| vu::mul(a[k], q)),
                0x1e => lanes(&|k| vu::mul(a[k], i)),
                0x20 => lanes(&|k| vu::add(a[k], q)),
                0x21 => lanes(&|k| vu::madd(acc[k], a[k], q)),
                0x22 => lanes(&|k| vu::add(a[k], i)),
                0x23 => lanes(&|k| vu::madd(acc[k], a[k], i)),
                0x24 => lanes(&|k| vu::sub(a[k], q)),
                0x25 => lanes(&|k| vu::msub(acc[k], a[k], q)),
                0x26 => lanes(&|k| vu::sub(a[k], i)),
                0x27 => lanes(&|k| vu::msub(acc[k], a[k], i)),
                0x28 => lanes(&|k| vu::add(a[k], b[k])),
                0x29 => lanes(&|k| vu::madd(acc[k], a[k], b[k])),
                0x2a => lanes(&|k| vu::mul(a[k], b[k])),
                0x2c => lanes(&|k| vu::sub(a[k], b[k])),
                0x2d => lanes(&|k| vu::msub(acc[k], a[k], b[k])),
                _ => return None,
            })
        };
        // max/mini compare like the FPU's, on the bit patterns
        let (max, min) = (|x: f32, y: f32| fpu::max(vu::operand(x.to_bits()), vu::operand(y.to_bits())),
                          |x: f32, y: f32| fpu::min(vu::operand(x.to_bits()), vu::operand(y.to_bits())));
        if fun < 0x3c {
            if let Some(v) = fmac(fun) {
                return Up::Write(Out::Vf(fd), dest, v, true);
            }
            let v = match fun {
                0x10..=0x13 => lanes(&|k| max(a[k], bc[k])),
                0x14..=0x17 => lanes(&|k| min(a[k], bc[k])),
                0x1d => lanes(&|k| max(a[k], i)),
                0x1f => lanes(&|k| min(a[k], i)),
                0x2b => lanes(&|k| max(a[k], b[k])),
                0x2f => lanes(&|k| min(a[k], b[k])),
                0x2e => return Up::Write(Out::Vf(fd), dest, lanes(&|k| vu::msub(acc[k], a[cy[k]], b[cz[k]])), true),
                _ => self.vu0_unknown(up, here, "upper op"),
            };
            return Up::Write(Out::Vf(fd), dest, v, false);
        }
        let op = (up >> 6 & 31) << 2 | fun & 3;
        match op {
            0x10..=0x13 => {
                let v = ps2_float::vu::Vf(a).itof_bits([0, 4, 12, 15][(op & 3) as usize]).0;
                Up::Write(Out::Vf(ft), dest, v, false)
            }
            0x14..=0x17 => {
                let v = ps2_float::vu::Vf(a).ftoi_bits([0, 4, 12, 15][(op & 3) as usize]).0;
                Up::Write(Out::Vf(ft), dest, v, false)
            }
            0x1d => Up::Write(Out::Vf(ft), dest, a.map(|x| f32::from_bits(x.to_bits() & 0x7fff_ffff)), false),
            0x1f => {
                let w = f32::from_bits(vu::operand(b[3].to_bits()).to_bits() & 0x7fff_ffff);
                let mut bits = 0;
                for k in 0..3 {
                    let x = vu::operand(a[k].to_bits());
                    if x > w {
                        bits |= 1 << (2 * k);
                    }
                    if x < -w {
                        bits |= 2 << (2 * k);
                    }
                }
                Up::Clip(bits)
            }
            0x2f => Up::Nothing,
            0x2e => Up::Write(Out::Acc, dest, lanes(&|k| vu::mul(a[cy[k]], b[cz[k]])), true),
            _ => match fmac(op) {
                Some(v) => Up::Write(Out::Acc, dest, v, true),
                None => self.vu0_unknown(up, here, "upper accumulator op"),
            },
        }
    }

    /// The registers a lower half reads (float lanes), and whether it waits for the divider.
    fn lower_reads(&self, l: u32) -> (Vec<(usize, [bool; 4])>, bool) {
        let (op, dest, ft, fs) = (l >> 25, l >> 21 & 15, (l >> 16 & 31) as usize, (l >> 11 & 31) as usize);
        let (fsf, ftf) = ((l >> 21 & 3) as usize, (l >> 23 & 3) as usize);
        let d = dest_lanes(dest);
        match op {
            0x01 => (vec![(fs, d)], false), // sq
            0x40 if l & 63 >= 0x3c => match (l >> 6 & 31) << 2 | l & 3 {
                0x30 | 0x35 | 0x37 => (vec![(fs, d)], false), // move, sqi, sqd
                0x31 => (vec![(fs, [true; 4])], false),        // mr32 reads the lanes rotated
                0x38 | 0x3a => (vec![(fs, one_lane(fsf)), (ft, one_lane(ftf))], true),
                0x39 => (vec![(ft, one_lane(ftf))], true),
                0x3b => (vec![], true),
                0x3c => (vec![(fs, one_lane(fsf))], false),
                _ => (vec![], false),
            },
            _ => (vec![], false),
        }
    }

    /// The lower half. Float register writes go out through `vf_out` (the upper half's write wins a clash);
    /// returns a branch target.
    #[allow(clippy::too_many_arguments)]
    fn lower(&mut self, p: &mut Pipe, l: u32, vf: &[[f32; 4]; 32], vi: &[u16; 16], issue: u64,
             vf_out: &mut Option<(usize, u32, [f32; 4])>, here: u32) -> Option<u32> {
        let op = l >> 25;
        let (dest, it, is) = (l >> 21 & 15, (l >> 16 & 15) as usize, (l >> 11 & 15) as usize);
        let (ft, fs) = ((l >> 16 & 31) as usize, (l >> 11 & 31) as usize);
        let imm11 = ((l & 0x7ff) as i32) << 21 >> 21;
        let imm12 = (l >> 10 & 0x800) | l & 0x7ff;
        let imm15 = ((l >> 10 & 0x7800) | l & 0x7ff) as u16;
        let imm24 = l & 0xff_ffff;
        let next = here + 1;
        let branch = (next as i32 + imm11) as u32;
        let set = |s: &mut Self, r: usize, v: u16| {
            if r != 0 {
                s.vi[r] = v;
            }
        };
        let flags = |s: &Self| (s.status & 0xfff, s.mac & 0xffff, s.clip & 0xff_ffff);
        match op {
            0x00 | 0x01 => {
                let at = vi[is].wrapping_add(imm11 as u16);
                if op == 0 {
                    *vf_out = Some((ft, dest, self.vu0_load(at, here)));
                } else {
                    self.vu0_store(vi[it].wrapping_add(imm11 as u16), dest, vf[fs], here);
                }
            }
            0x04 => {
                let q = self.vu0_load(vi[is].wrapping_add(imm11 as u16), here);
                let k = (0..4).find(|&k| has(dest, k)).unwrap_or(0);
                set(self, it, q[k].to_bits() as u16);
            }
            0x05 => {
                let v = f32::from_bits(vi[it] as u32);
                self.vu0_store(vi[is].wrapping_add(imm11 as u16), dest, [v; 4], here);
            }
            0x08 => set(self, it, vi[is].wrapping_add(imm15)),
            0x09 => set(self, it, vi[is].wrapping_sub(imm15)),
            0x10 => set(self, 1, (flags(self).2 == imm24) as u16),
            0x11 => self.clip = imm24,
            0x12 => set(self, 1, (flags(self).2 & imm24 != 0) as u16),
            0x13 => set(self, 1, ((flags(self).2 | imm24) == 0xff_ffff) as u16),
            0x14 => set(self, it, (flags(self).0 == imm12) as u16),
            0x15 => self.status = self.status & 0x3f | imm12 & 0xfc0,
            0x16 => set(self, it, (flags(self).0 & imm12) as u16),
            0x17 => set(self, it, (flags(self).0 | imm12) as u16),
            0x18 => set(self, it, (flags(self).1 == vi[is] as u32) as u16),
            0x1a => set(self, it, (flags(self).1 & vi[is] as u32) as u16),
            0x1b => set(self, it, (flags(self).1 | vi[is] as u32) as u16),
            0x1c => set(self, it, (flags(self).2 & 0xfff) as u16),
            0x20 => return Some(branch),
            0x21 => {
                set(self, it, (here + 2) as u16);
                return Some(branch);
            }
            0x24 => return Some(vi[is] as u32),
            0x25 => {
                set(self, it, (here + 2) as u16);
                return Some(vi[is] as u32);
            }
            0x28..=0x2f => {
                let (s, t) = (vi[is] as i16, vi[it] as i16);
                let take = match op {
                    0x28 => s == t,
                    0x29 => s != t,
                    0x2c => s < 0,
                    0x2d => s > 0,
                    0x2e => s <= 0,
                    0x2f => s >= 0,
                    _ => self.vu0_unknown(l, here, "lower branch"),
                };
                return take.then_some(branch);
            }
            0x40 => {
                let fun = l & 63;
                if fun < 0x3c {
                    let imm5 = ((l >> 6 & 31) as i32) << 27 >> 27;
                    let id = (l >> 6 & 15) as usize;
                    let v = match fun {
                        0x30 => vi[is].wrapping_add(vi[it]),
                        0x31 => vi[is].wrapping_sub(vi[it]),
                        0x32 => {
                            set(self, it, vi[is].wrapping_add(imm5 as u16));
                            return None;
                        }
                        0x34 => vi[is] & vi[it],
                        0x35 => vi[is] | vi[it],
                        _ => self.vu0_unknown(l, here, "lower integer op"),
                    };
                    set(self, id, v);
                    return None;
                }
                let (fsf, ftf) = ((l >> 21 & 3) as usize, (l >> 23 & 3) as usize);
                match (l >> 6 & 31) << 2 | fun & 3 {
                    0x30 => *vf_out = Some((ft, dest, vf[fs])), // move (and nop, as move.0 vf0, vf0)
                    0x31 => *vf_out = Some((ft, dest, [vf[fs][1], vf[fs][2], vf[fs][3], vf[fs][0]])), // mr32
                    0x34 => {
                        *vf_out = Some((ft, dest, self.vu0_load(vi[is], here))); // lqi
                        set(self, is, vi[is].wrapping_add(1));
                    }
                    0x35 => {
                        self.vu0_store(vi[it], dest, vf[fs], here); // sqi
                        set(self, it, vi[it].wrapping_add(1));
                    }
                    0x36 => {
                        let at = vi[is].wrapping_sub(1); // lqd
                        set(self, is, at);
                        *vf_out = Some((ft, dest, self.vu0_load(at, here)));
                    }
                    0x37 => {
                        let at = vi[it].wrapping_sub(1); // sqd
                        set(self, it, at);
                        self.vu0_store(at, dest, vf[fs], here);
                    }
                    k @ 0x38..=0x3a => {
                        let (a, b) = (vf[fs][fsf], vf[ft][ftf]);
                        let (a, b) = (vu::operand(a.to_bits()), vu::operand(b.to_bits()));
                        // the divider's status bits: I for 0/0 (and sqrt of a negative), D for x/0
                        let (q, id, lat) = match k {
                            0x38 => (vu::div(a, b), if b == 0.0 { if a == 0.0 { 0x10 } else { 0x20 } } else { 0 }, DIV),
                            0x39 => (vu::sqrt(b), if b < 0.0 { 0x10 } else { 0 }, DIV),
                            _ => (vu::rsqrt(a, b), if b == 0.0 { 0x20 } else if b < 0.0 { 0x10 } else { 0 }, RSQRT),
                        };
                        p.q = Some((q, issue + lat, id));
                    }
                    0x3b => {} // waitq: the stall happened before issue
                    0x3c => set(self, it, vf[fs][fsf].to_bits() as u16), // mtir
                    0x3d => *vf_out = Some((ft, dest, [f32::from_bits(vi[is] as i16 as i32 as u32); 4])), // mfir
                    0x3e => {
                        let q = self.vu0_load(vi[is], here); // ilwr
                        let k = (0..4).find(|&k| has(dest, k)).unwrap_or(0);
                        set(self, it, q[k].to_bits() as u16);
                    }
                    0x3f => self.vu0_store(vi[is], dest, [f32::from_bits(vi[it] as u32); 4], here), // iswr
                    _ => self.vu0_unknown(l, here, "lower op (VU1-only, EFU or R register)"),
                }
            }
            _ => self.vu0_unknown(l, here, "lower op"),
        }
        None
    }

    /// A quadword of VU0 data memory at a quadword address (it wraps at 4 KB).
    fn vu0_load(&self, at: u16, here: u32) -> [f32; 4] {
        let o = self.vu0_qaddr(at, here);
        std::array::from_fn(|k| f32::from_bits(u32::from_le_bytes(self.vu0_data[o + 4 * k..o + 4 * k + 4].try_into().unwrap())))
    }

    fn vu0_store(&mut self, at: u16, dest: u32, v: [f32; 4], here: u32) {
        let o = self.vu0_qaddr(at, here);
        for k in 0..4 {
            if has(dest, k) {
                self.vu0_data[o + 4 * k..o + 4 * k + 4].copy_from_slice(&v[k].to_bits().to_le_bytes());
            }
        }
    }

    fn vu0_qaddr(&self, at: u16, here: u32) -> usize {
        assert!(at & 0x400 == 0, "VU0 micro program at {here:#x}: data address {at:#x} is VU1's register window, \
                                  which the interpreter does not have");
        (at as usize * 16) % MEM
    }

    fn vu0_unknown(&self, w: u32, here: u32, what: &str) -> ! {
        panic!("unimplemented VU0 micro {what} {w:#010x} at {here:#x} (8-byte units)\n\
                Teach it to ps2-oracle's src/vu0.rs (FIZL0/ps2-oracle).");
    }
}
