//! The interpreter: an R5900 (PS2 Emotion Engine) core that runs one function and stops.
//!
//! It is not an emulator. There is no DMA (beyond replaying a VIF0 upload chain), no GS, no interrupt, no timer, no
//! VU1, no IOP and no clock:
//! just registers, RAM and the instruction set, enough to call a leaf-ish function and read its answer.
//! That is a deliberate trade — it makes a call cost microseconds instead of a frame, so a test can make
//! ten thousand of them, which is the difference between "my port looks right" and "my port is right".
//!
//! Arithmetic comes from `ps2-float`, so the oracle and the port under test are proved against the *same*
//! model of the hardware, and a difference between them can only be in the logic.

use ps2_float::{fpu, vu};
use std::collections::{BTreeMap, BTreeSet};

/// The return address planted in `ra`: when `pc` reaches it, the call is over. Chosen outside RAM so a
/// stray jump to it cannot be mistaken for real code.
pub const DONE: u32 = 0xffff_fff0;

/// Size of the EE's scratchpad (`0x70000000`..), which some compiled code uses as a fast local area.
const SPR: usize = 0x4000;

/// A Rust function standing in for one of the game's: see [`Ee::hooks`].
pub type Hook = Box<dyn FnMut(&mut Ee)>;

/// Where an address lands: see [`Ee::loc`].
enum Mem {
    Ram,
    Spr,
    Micro0,
    Data0,
}

/// One R5900 core with its memory.
pub struct Ee {
    /// The 32 general-purpose registers, 128 bits each (ordinary code only touches the low 64).
    pub gpr: [u128; 32],
    pub hi: u64,
    pub lo: u64,
    /// The second multiplier's result registers (`mult1`, `div1`, `mfhi1`…).
    pub hi1: u64,
    pub lo1: u64,
    /// COP1 registers as raw bit patterns, because `mfc1`/`mtc1` move bits, not values.
    pub fpr: [u32; 32],
    /// The FPU's accumulator (`adda.s`, `madda.s`, …).
    pub acc: u32,
    /// The FPU's single compare flag, which `bc1t`/`bc1f` branch on.
    pub cond: bool,
    /// VU0's float registers. `vf0` reads as `(0, 0, 0, 1)` and cannot be written.
    pub vf: [[f32; 4]; 32],
    /// VU0's accumulator and its `Q` register (`vdiv`, `vsqrt`, `vrsqrt` land in `Q`).
    pub vacc: [f32; 4],
    pub q: f32,
    /// VU0's integer registers (`vi0` reads as zero), its `I` register and its flags: the MAC flags
    /// (`Z`/`S`/`U`/`O` per lane), the status flags (sticky bits included) and the clipping history. Only
    /// micro mode ([`Ee::vu0_run`]) reads and writes them; macro-mode arithmetic leaves the flags alone.
    pub vi: [u16; 16],
    pub i: f32,
    pub mac: u32,
    pub status: u32,
    pub clip: u32,
    /// VU0's 4 KB of micro (program) memory and 4 KB of data memory, which the EE also sees at
    /// `0x11000000` and `0x11004000`. A game fills the micro memory with a VIF0 `MPG` ([`Ee::vif0_dma`]).
    pub vu0_micro: Vec<u8>,
    pub vu0_data: Vec<u8>,
    /// VU0 cycles the last micro program took, stalls included.
    pub vu0_cycles: u64,
    /// Where `vcallmsr` starts a micro program (VU0 control register 27), in 8-byte units.
    pub cmsar0: u32,
    pub pc: u32,
    pub npc: u32,
    /// EE RAM (32 MB on a retail console; 128 MB on a development unit).
    pub ram: Vec<u8>,
    /// The scratchpad at `0x70000000`.
    pub spr: Vec<u8>,
    /// Every byte address written since the current call began — how a test checks out-parameters and
    /// how a tool reports what a function touched.
    pub written: BTreeSet<u32>,
    /// Instructions executed by the last call.
    pub steps: u64,
    /// The stack pointer each call starts with. Defaults to 16 KB below the top of RAM; move it if a RAM
    /// dump you loaded has live data there.
    pub stack: u32,
    /// Functions replaced by Rust closures: reaching the address runs the closure (which reads its
    /// arguments out of the registers and writes its result back) and returns to `ra`.
    ///
    /// Three uses: stubbing out something the interpreter cannot do (a VU1 micro program, a DMA wait),
    /// cutting a dependency on a subsystem you have not ported yet, and *recording* — a hook that only
    /// logs its arguments and then runs nothing turns the oracle into a tracer.
    pub hooks: BTreeMap<u32, Hook>,
    /// When not empty, the only RAM a program may touch, as `(start, end)` ranges, and every access must be
    /// aligned to its size (`lq`/`sq`/`lqc2`/`sqc2` included, which the console would round down): what a
    /// caller that only wants the behaviour of valid states uses to fault on the others. Empty by default.
    pub strict: Vec<(u32, u32)>,
}

impl Ee {
    /// A machine with `ram` bytes of zeroed RAM and nothing loaded. Useful on its own for running
    /// hand-assembled code in a test; see [`Ee::load`].
    pub fn bare(ram: usize) -> Ee {
        Ee::with_ram(vec![0; ram])
    }

    /// A machine over an existing RAM image (a save state's EE memory, or a boot image you built).
    pub fn with_ram(ram: Vec<u8>) -> Ee {
        let stack = (ram.len() as u32).saturating_sub(0x4000);
        Ee {
            gpr: [0; 32],
            hi: 0,
            lo: 0,
            hi1: 0,
            lo1: 0,
            fpr: [0; 32],
            acc: 0,
            cond: false,
            vf: std::array::from_fn(|k| if k == 0 { [0.0, 0.0, 0.0, 1.0] } else { [0.0; 4] }),
            vacc: [0.0; 4],
            q: 0.0,
            vi: [0; 16],
            i: 0.0,
            mac: 0,
            status: 0,
            clip: 0,
            vu0_micro: vec![0; crate::vu0::MEM],
            vu0_data: vec![0; crate::vu0::MEM],
            vu0_cycles: 0,
            cmsar0: 0,
            pc: 0,
            npc: 0,
            ram,
            spr: vec![0; SPR],
            written: BTreeSet::new(),
            steps: 0,
            stack,
            hooks: BTreeMap::new(),
            strict: Vec::new(),
        }
    }

    /// Copy bytes into RAM at a virtual address (what a loader does).
    pub fn load(&mut self, va: u32, bytes: &[u8]) {
        let at = (va & 0x0fff_ffff) as usize;
        let end = at + bytes.len();
        assert!(end <= self.ram.len(),
                "a program wants {va:#x}..{:#x}, past the end of {} MB of RAM — check the RAM size and the \
                 load addresses", va as usize + bytes.len(), self.ram.len() >> 20);
        self.ram[at..end].copy_from_slice(bytes);
    }

    /// Call `addr` with integer arguments in `a0..a7` and float arguments in `f12..f19`, and run until it
    /// returns.
    ///
    /// Panics after `budget` instructions, which is a feature: a function that needs the hardware to make
    /// progress (waiting on a DMA, on a VU, on a timer) spins instead of returning, and the panic says
    /// where. Read the results with [`Ee::v0`], [`Ee::f0`], [`Ee::written`] and memory reads.
    ///
    /// The MIPS o32-style convention PS2 compilers use: `a0..a3` in `$4..$7` (`a4..a7` in `$8..$11` for
    /// the EE's extended ABI), floats in `$f12` up, integer result in `v0`, float result in `$f0`, and the
    /// caller leaves 16 bytes of argument space on the stack, which this does.
    pub fn call(&mut self, addr: u32, ints: &[u64], floats: &[f32], budget: u64) {
        for (k, &v) in ints.iter().enumerate() {
            assert!(k < 8, "more than 8 integer arguments: the rest go on the stack");
            self.set(4 + k, v);
        }
        for (k, &v) in floats.iter().enumerate() {
            assert!(k < 8, "more than 8 float arguments: the rest go on the stack");
            self.fpr[12 + k] = v.to_bits();
        }
        self.set(29, sx32(self.stack)); // sp
        self.set(31, sx32(DONE)); // ra
        self.pc = addr;
        self.npc = addr.wrapping_add(4);
        self.written.clear();
        self.steps = 0;
        while self.pc != DONE {
            assert!(self.steps < budget,
                    "call {addr:#08x}: still running after {budget} instructions (pc {:#08x}). Either the \
                     budget is too low, or it is waiting for hardware this interpreter does not have — \
                     hook the function that waits.", self.pc);
            if let Some(mut h) = self.hooks.remove(&self.pc) {
                h(self);
                self.hooks.insert(self.pc, h);
                self.pc = self.r(31) as u32;
                self.npc = self.pc.wrapping_add(4);
                continue;
            }
            self.step();
            self.steps += 1;
        }
    }

    /// The integer return value (`v0`).
    pub fn v0(&self) -> u64 {
        self.r(2)
    }
    /// The float return value (`$f0`).
    pub fn f0(&self) -> f32 {
        f32::from_bits(self.fpr[0])
    }
    /// One register's low 64 bits.
    pub fn r(&self, i: usize) -> u64 {
        self.gpr[i] as u64
    }
    /// Write a register's low 64 bits; the upper 64 are kept, as on the R5900. `$0` stays zero.
    pub fn set(&mut self, i: usize, v: u64) {
        if i != 0 {
            self.gpr[i] = (self.gpr[i] & !(u64::MAX as u128)) | v as u128;
        }
    }
    /// One FPU register as a float.
    pub fn f(&self, i: usize) -> f32 {
        f32::from_bits(self.fpr[i])
    }
    /// Set one FPU register from a float.
    pub fn set_f(&mut self, i: usize, v: f32) {
        self.fpr[i] = v.to_bits();
    }

    /// Which memory a virtual address is in, and the offset. Everything the EE's two cached mappings
    /// cover lands in RAM; a dangling pointer panics here rather than reading nonsense.
    fn loc(&self, a: u32) -> (Mem, usize) {
        let vu = crate::vu0::MEM as u32;
        if a >> 28 == 7 {
            (Mem::Spr, (a & (SPR as u32 - 1)) as usize)
        } else if (a & 0x0fff_ffff) < self.ram.len() as u32 && a & 0x1000_0000 == 0 {
            (Mem::Ram, (a & 0x0fff_ffff) as usize)
        } else if a.wrapping_sub(0x1100_0000) < vu {
            (Mem::Micro0, (a - 0x1100_0000) as usize)
        } else if a.wrapping_sub(0x1100_4000) < vu {
            (Mem::Data0, (a - 0x1100_4000) as usize)
        } else {
            panic!("access to {a:#010x} (pc {:#08x}) is not RAM or scratchpad. A pointer argument is \
                    probably wrong, or the function needs state a RAM dump would have.",
                   self.pc.wrapping_sub(4))
        }
    }
    /// `n` bytes at `a`, little-endian.
    fn strict(&self, a: u32, n: usize) {
        let r = a & 0x0fff_ffff;
        if !self.strict.is_empty() && (a as usize % n != 0
            || a >> 28 != 7 && !self.strict.iter().any(|&(s, e)| (s..e).contains(&r))) {
            panic!("access to {a:#010x} (pc {:#08x}) faults: low or misaligned", self.pc.wrapping_sub(4));
        }
    }
    pub fn read(&self, a: u32, n: usize) -> u128 {
        self.strict(a, n);
        let (m, o) = self.loc(a);
        let m = match m {
            Mem::Ram => &self.ram,
            Mem::Spr => &self.spr,
            Mem::Micro0 => &self.vu0_micro,
            Mem::Data0 => &self.vu0_data,
        };
        (0..n).fold(0u128, |v, k| v | (m[o + k] as u128) << (8 * k))
    }
    /// Write `n` bytes at `a`, little-endian, and remember that they changed.
    pub fn write(&mut self, a: u32, n: usize, v: u128) {
        self.strict(a, n);
        let (m, o) = self.loc(a);
        let m = match m {
            Mem::Ram => &mut self.ram,
            Mem::Spr => &mut self.spr,
            Mem::Micro0 => &mut self.vu0_micro,
            Mem::Data0 => &mut self.vu0_data,
        };
        for k in 0..n {
            m[o + k] = (v >> (8 * k)) as u8;
        }
        self.written.extend(a..a.wrapping_add(n as u32));
    }
    pub fn u8_at(&self, a: u32) -> u8 {
        self.read(a, 1) as u8
    }
    pub fn u16_at(&self, a: u32) -> u16 {
        self.read(a, 2) as u16
    }
    pub fn u32_at(&self, a: u32) -> u32 {
        self.read(a, 4) as u32
    }
    pub fn i32_at(&self, a: u32) -> i32 {
        self.read(a, 4) as u32 as i32
    }
    /// A float in memory — as the FPU would read it (Inf/NaN as `±max`, denormals as `±0`).
    pub fn f32_at(&self, a: u32) -> f32 {
        fpu::operand(self.read(a, 4) as u32)
    }
    pub fn set_u32(&mut self, a: u32, v: u32) {
        self.write(a, 4, v as u128);
    }
    pub fn set_f32(&mut self, a: u32, v: f32) {
        self.write(a, 4, v.to_bits() as u128);
    }
    /// A scratch buffer inside the stack region, for out-parameters: the address stays clear of the
    /// space a call uses for its own frame.
    pub fn scratch(&self, k: u32) -> u32 {
        self.stack + 0x40 + k * 0x40
    }

    /// What the last call wrote, merged into `(start, end)` ranges — short enough to print.
    pub fn written_ranges(&self) -> Vec<(u32, u32)> {
        let mut out: Vec<(u32, u32)> = Vec::new();
        for &a in &self.written {
            match out.last_mut() {
                Some(last) if last.1 == a => last.1 = a + 1,
                _ => out.push((a, a + 1)),
            }
        }
        out
    }

    fn branch(&mut self, take: bool, target: u32, likely: bool) {
        if take {
            self.npc = target;
        } else if likely {
            self.pc = self.npc; // a "likely" branch not taken skips its delay slot
            self.npc = self.npc.wrapping_add(4);
        }
    }

    /// One instruction, delay slot and all.
    pub fn step(&mut self) {
        let w = self.read(self.pc, 4) as u32;
        let here = self.pc;
        self.pc = self.npc;
        self.npc = self.npc.wrapping_add(4);
        let (op, rs, rt, rd, sa, fun) = (w >> 26, (w >> 21 & 31) as usize, (w >> 16 & 31) as usize,
                                         (w >> 11 & 31) as usize, w >> 6 & 31, w & 63);
        let imm = w as u16 as i16 as i64 as u64;
        let (s, t) = (self.r(rs), self.r(rt));
        let ea = (s as u32).wrapping_add(imm as u32);
        let bt = self.pc.wrapping_add((imm << 2) as u32);
        match op {
            0 => match fun {
                0 => self.set(rd, sx32((t as u32) << sa)),
                2 => self.set(rd, sx32((t as u32) >> sa)),
                3 => self.set(rd, sx32(((t as i32) >> sa) as u32)),
                4 => self.set(rd, sx32((t as u32) << (s & 31))),
                6 => self.set(rd, sx32((t as u32) >> (s & 31))),
                7 => self.set(rd, sx32(((t as i32) >> (s & 31)) as u32)),
                8 => self.npc = s as u32, // jr
                9 => {
                    self.set(rd, sx32(self.pc.wrapping_add(4))); // jalr
                    self.npc = s as u32;
                }
                0xa if t == 0 => self.set(rd, s), // movz
                0xb if t != 0 => self.set(rd, s), // movn
                0xa | 0xb | 0xf => {}             // the untaken moves, and sync
                0x10 => self.set(rd, self.hi),
                0x11 => self.hi = s,
                0x12 => self.set(rd, self.lo),
                0x13 => self.lo = s,
                0x14 => self.set(rd, t << (s & 63)),
                0x16 => self.set(rd, t >> (s & 63)),
                0x17 => self.set(rd, ((t as i64) >> (s & 63)) as u64),
                0x18 | 0x19 => {
                    let p = if fun == 0x18 {
                        (s as i32 as i64 * t as i32 as i64) as u64
                    } else {
                        (s as u32 as u64) * (t as u32 as u64)
                    };
                    (self.lo, self.hi) = (sx32(p as u32), sx32((p >> 32) as u32));
                    self.set(rd, self.lo); // the EE's mult also writes rd
                }
                0x1a => (self.lo, self.hi) = idiv(s as u32, t as u32),
                0x1b => (self.lo, self.hi) = udiv(s as u32, t as u32),
                0x20 | 0x21 => self.set(rd, sx32((s as u32).wrapping_add(t as u32))),
                0x22 | 0x23 => self.set(rd, sx32((s as u32).wrapping_sub(t as u32))),
                0x24 => self.set(rd, s & t),
                0x25 => self.set(rd, s | t),
                0x26 => self.set(rd, s ^ t),
                0x27 => self.set(rd, !(s | t)),
                0x2a => self.set(rd, ((s as i64) < (t as i64)) as u64),
                0x2b => self.set(rd, (s < t) as u64),
                0x2c | 0x2d => self.set(rd, s.wrapping_add(t)),
                0x2e | 0x2f => self.set(rd, s.wrapping_sub(t)),
                0x38 => self.set(rd, t << sa),
                0x3a => self.set(rd, t >> sa),
                0x3b => self.set(rd, ((t as i64) >> sa) as u64),
                0x3c => self.set(rd, t << (sa + 32)),
                0x3e => self.set(rd, t >> (sa + 32)),
                0x3f => self.set(rd, ((t as i64) >> (sa + 32)) as u64),
                _ => self.unknown(w, here, "SPECIAL"),
            },
            1 => {
                let take = if rt & 1 == 0 { (s as i64) < 0 } else { (s as i64) >= 0 };
                if rt & 0x10 != 0 {
                    self.set(31, sx32(self.pc.wrapping_add(4)));
                }
                match rt {
                    0 | 1 | 0x10 | 0x11 => self.branch(take, bt, false),
                    2 | 3 | 0x12 | 0x13 => self.branch(take, bt, true),
                    _ => self.unknown(w, here, "REGIMM"),
                }
            }
            2 | 3 => {
                if op == 3 {
                    self.set(31, sx32(self.pc.wrapping_add(4)));
                }
                self.npc = (self.pc & 0xf000_0000) | (w & 0x03ff_ffff) << 2;
            }
            4 | 0x14 => self.branch(s == t, bt, op == 0x14),
            5 | 0x15 => self.branch(s != t, bt, op == 0x15),
            6 | 0x16 => self.branch(s as i64 <= 0, bt, op == 0x16),
            7 | 0x17 => self.branch(s as i64 > 0, bt, op == 0x17),
            8 | 9 => self.set(rt, sx32((s as u32).wrapping_add(imm as u32))),
            0xa => self.set(rt, ((s as i64) < imm as i64) as u64),
            0xb => self.set(rt, (s < imm) as u64),
            0xc => self.set(rt, s & (w & 0xffff) as u64),
            0xd => self.set(rt, s | (w & 0xffff) as u64),
            0xe => self.set(rt, s ^ (w & 0xffff) as u64),
            0xf => self.set(rt, sx32(w << 16)),
            0x11 => self.cop1(w, here),
            0x12 => self.cop2(w, here),
            0x18 | 0x19 => self.set(rt, s.wrapping_add(imm)), // daddi, daddiu
            0x1a | 0x1b | 0x22 | 0x26 => self.load_lr(op, rt, ea),
            0x1c => self.mmi(w, here),
            0x1e => {
                self.strict(ea, 16);
                let v = self.read(ea & !15, 16); // lq
                if rt != 0 {
                    self.gpr[rt] = v;
                }
            }
            0x1f => {
                self.strict(ea, 16);
                self.write(ea & !15, 16, self.gpr[rt]) // sq
            }
            0x20 => self.set(rt, self.read(ea, 1) as u8 as i8 as i64 as u64),
            0x21 => self.set(rt, self.read(ea, 2) as u16 as i16 as i64 as u64),
            0x23 => self.set(rt, sx32(self.read(ea, 4) as u32)),
            0x24 => self.set(rt, self.read(ea, 1) as u64),
            0x25 => self.set(rt, self.read(ea, 2) as u64),
            0x27 => self.set(rt, self.read(ea, 4) as u64),
            0x28 => self.write(ea, 1, t as u128),
            0x29 => self.write(ea, 2, t as u128),
            0x2a | 0x2c | 0x2d | 0x2e => self.store_lr(op, t, ea),
            0x2b => self.write(ea, 4, t as u128),
            0x2f | 0x33 => {} // cache, pref: no caches here
            0x31 => self.fpr[rt] = self.read(ea, 4) as u32, // lwc1
            0x36 => {
                self.strict(ea, 16);
                let v = self.read(ea & !15, 16); // lqc2
                self.put_vf(rt, 0xf, std::array::from_fn(|k| f32::from_bits((v >> (32 * k)) as u32)));
            }
            0x37 => self.set(rt, self.read(ea, 8) as u64),
            0x39 => self.write(ea, 4, self.fpr[rt] as u128), // swc1
            0x3e => {
                let v = self.vf[rt].iter().enumerate() // sqc2
                    .fold(0u128, |a, (k, f)| a | (f.to_bits() as u128) << (32 * k));
                self.strict(ea, 16);
                self.write(ea & !15, 16, v);
            }
            0x3f => self.write(ea, 8, t as u128),
            _ => self.unknown(w, here, "opcode"),
        }
    }

    /// The contract for anything unimplemented: say exactly what and where, and stop.
    ///
    /// This is the single most useful message the oracle produces. An unknown instruction means the
    /// function you called does something the interpreter has not been taught yet; the fix is to add it
    /// here (a dozen lines, in the matching `match`), not to work around it. The word is printed so you
    /// can decode it with any R5900 disassembler.
    fn unknown(&self, w: u32, here: u32, what: &str) -> ! {
        panic!("unimplemented {what} {w:#010x} at {here:#08x}\n\
                Teach it to ps2-oracle's src/ee.rs (see the match arms near this one; FIZL0/ps2-oracle), or \
                hook the function that uses it.");
    }

    /// `lwl` / `lwr` / `ldl` / `ldr`: the unaligned loads a compiler emits for packed structs and for
    /// `memcpy` of odd lengths. Little-endian, so "left" is the high end of the word.
    fn load_lr(&mut self, op: u32, rt: usize, ea: u32) {
        let n = if op >= 0x22 { 4 } else { 8 };
        let (al, sh) = (ea & !(n as u32 - 1), (ea as usize) & (n - 1));
        let left = op == 0x22 || op == 0x1a;
        let mut v = self.r(rt).to_le_bytes();
        for i in 0..n {
            if left && i <= sh {
                v[n - 1 - sh + i] = self.read(al + i as u32, 1) as u8;
            } else if !left && i >= sh {
                v[i - sh] = self.read(al + i as u32, 1) as u8;
            }
        }
        let v = u64::from_le_bytes(v);
        // a 32-bit form that fills the whole word sign-extends it (`lwl`, and `lwr` from an aligned address);
        // `lwr` filling only the low bytes keeps the register's upper half, as the R5900 does
        self.set(rt, if n == 4 && (left || sh == 0) { sx32(v as u32) } else { v });
    }

    fn store_lr(&mut self, op: u32, t: u64, ea: u32) {
        let n = if op == 0x2a || op == 0x2e { 4 } else { 8 };
        let (al, sh) = (ea & !(n as u32 - 1), (ea as usize) & (n - 1));
        let left = op == 0x2a || op == 0x2c;
        let b = t.to_le_bytes();
        for i in 0..n {
            if left && i <= sh {
                self.write(al + i as u32, 1, b[n - 1 - sh + i] as u128);
            } else if !left && i >= sh {
                self.write(al + i as u32, 1, b[i - sh] as u128);
            }
        }
    }

    /// COP1: the scalar FPU, every result through [`ps2_float::fpu`].
    fn cop1(&mut self, w: u32, here: u32) {
        let (fmt, ft, fs, fdn, fun) = (w >> 21 & 31, (w >> 16 & 31) as usize, (w >> 11 & 31) as usize,
                                       (w >> 6 & 31) as usize, w & 63);
        match fmt {
            0 => self.set(ft, sx32(self.fpr[fs])),  // mfc1
            2 => self.set(ft, sx32(if fs == 31 { 0x0100_0001 | (self.cond as u32) << 23 } else { 0x2e00 })), // cfc1
            4 => self.fpr[fs] = self.r(ft) as u32,  // mtc1
            6 => {}                                 // ctc1: no status bits to set (one rounding mode)
            8 => {
                let bt = self.pc.wrapping_add(((w as u16 as i16 as i32) << 2) as u32);
                self.branch(self.cond == (ft & 1 == 1), bt, ft & 2 != 0); // bc1f/bc1t (+l)
            }
            0x10 => {
                let (a, b, acc) = (fpu::operand(self.fpr[fs]), fpu::operand(self.fpr[ft]), fpu::operand(self.acc));
                let (sb, tb) = (self.fpr[fs], self.fpr[ft]);
                let put = |e: &mut Self, v: f32| e.fpr[fdn] = v.to_bits();
                match fun {
                    0 => put(self, fpu::add(a, b)),
                    1 => put(self, fpu::sub(a, b)),
                    2 => put(self, fpu::mul(a, b)),
                    3 => put(self, fpu::div(a, b)),
                    4 => put(self, fpu::sqrt(b)),
                    5 => self.fpr[fdn] = sb & 0x7fff_ffff, // abs.s
                    6 => self.fpr[fdn] = sb,               // mov.s
                    7 => self.fpr[fdn] = sb ^ 0x8000_0000, // neg.s
                    0x16 => {
                        // rsqrt.s: a / sqrt(b), and a zero divisor gives ±max rather than a trap
                        if tb & 0x7f80_0000 == 0 {
                            self.fpr[fdn] = ((sb ^ tb) & 0x8000_0000) | 0x7f7f_ffff;
                        } else {
                            put(self, fpu::rsqrt(a, b));
                        }
                    }
                    0x18 => self.acc = fpu::add(a, b).to_bits(),
                    0x19 => self.acc = fpu::sub(a, b).to_bits(),
                    0x1a => self.acc = fpu::mul(a, b).to_bits(),
                    0x1c => put(self, fpu::madd(acc, a, b)),
                    0x1d => put(self, fpu::msub(acc, a, b)),
                    0x1e => self.acc = fpu::madd(acc, a, b).to_bits(),
                    0x1f => self.acc = fpu::msub(acc, a, b).to_bits(),
                    0x24 => self.fpr[fdn] = fpu::to_i32(a) as u32, // cvt.w.s, truncating and saturating
                    0x28 => put(self, fpu::max(a, b)),
                    0x29 => put(self, fpu::min(a, b)),
                    0x30 => self.cond = false, // c.f.s
                    0x32 => self.cond = a == b,
                    0x34 => self.cond = a < b,
                    0x36 => self.cond = a <= b,
                    _ => self.unknown(w, here, "COP1 single-precision op"),
                }
            }
            0x14 if fun == 0x20 => self.fpr[fdn] = fpu::from_i32(self.fpr[fs] as i32).to_bits(), // cvt.s.w
            _ => self.unknown(w, here, "COP1 op"),
        }
    }

    /// Write the `dest` lanes (bit 3 = x … bit 0 = w) of a VU register. `vf0` is hard-wired.
    fn put_vf(&mut self, r: usize, dest: u32, v: [f32; 4]) {
        for (k, &lane) in v.iter().enumerate() {
            if r != 0 && dest >> (3 - k) & 1 == 1 {
                self.vf[r][k] = lane;
            }
        }
    }

    /// COP2: VU0 in macro mode — the vector instructions the EE issues inline, through
    /// [`ps2_float::vu`] — and the doors into micro mode: `vcallms`/`vcallmsr` run a program in VU0's
    /// micro memory ([`Ee::vu0_run`]) to its end before the next EE instruction, so the interlocking
    /// moves (`qmfc2.i`, `cfc2.i`) after it see the finished results, as they would after waiting.
    fn cop2(&mut self, w: u32, here: u32) {
        let (k, rt, rd) = (w >> 21 & 31, (w >> 16 & 31) as usize, (w >> 11 & 31) as usize);
        if k < 16 {
            match k {
                2 => {
                    // cfc2: a VU0 integer or control register, sign-extended
                    let v = match rd {
                        0..=15 => self.vi[rd] as u32,
                        16 => self.status,
                        17 => self.mac,
                        18 => self.clip,
                        21 => self.i.to_bits(),
                        22 => self.q.to_bits(),
                        27 => self.cmsar0,
                        _ => self.unknown(w, here, "cfc2 of this VU0 control register"),
                    };
                    self.set(rt, sx32(v));
                }
                6 => {
                    // ctc2: the status register keeps its live flags; only the sticky ones are written
                    let v = self.r(rt) as u32;
                    match rd {
                        0 => {}
                        1..=15 => self.vi[rd] = v as u16,
                        16 => self.status = self.status & 0x3f | v & 0xfc0,
                        17 => {}
                        18 => self.clip = v & 0xff_ffff,
                        21 => self.i = f32::from_bits(v),
                        22 => self.q = f32::from_bits(v),
                        27 => self.cmsar0 = v & 0xffff,
                        _ => self.unknown(w, here, "ctc2 to this VU0 control register"),
                    }
                }
                1 => {
                    // qmfc2: a whole vf register into a 128-bit GPR
                    let v = self.vf[rd].iter().enumerate()
                        .fold(0u128, |a, (k, f)| a | (f.to_bits() as u128) << (32 * k));
                    if rt != 0 {
                        self.gpr[rt] = v;
                    }
                }
                5 => {
                    let v = self.gpr[rt]; // qmtc2
                    self.put_vf(rd, 0xf, std::array::from_fn(|k| f32::from_bits((v >> (32 * k)) as u32)));
                }
                _ => self.unknown(w, here, "COP2 move"),
            }
            return;
        }
        let (dest, ft, fs, fdn, fun) = (w >> 21 & 15, (w >> 16 & 31) as usize, (w >> 11 & 31) as usize,
                                        (w >> 6 & 31) as usize, w & 63);
        let (a, b, acc, q) = (self.vf[fs], self.vf[ft], self.vacc, self.q);
        // the broadcast forms (vaddx, vmuly, …) read one lane of ft in all four lanes
        let bc = [b[(w & 3) as usize]; 4];
        let lanes = |f: &dyn Fn(usize) -> f32| -> [f32; 4] { std::array::from_fn(f) };
        // the cross-product pair: both halves use the same lane pattern, and the compiler swaps the
        // operands between them (see ps2_float::Vf::opmsub)
        let (cy, cz) = ([1, 2, 0, 3], [2, 0, 1, 3]);
        let out = match fun {
            0..=3 => lanes(&|i| vu::add(a[i], bc[i])),
            4..=7 => lanes(&|i| vu::sub(a[i], bc[i])),
            8..=11 => lanes(&|i| vu::madd(acc[i], a[i], bc[i])),
            12..=15 => lanes(&|i| vu::msub(acc[i], a[i], bc[i])),
            0x18..=0x1b => lanes(&|i| vu::mul(a[i], bc[i])),
            0x10..=0x13 => lanes(&|i| if a[i] > bc[i] { a[i] } else { bc[i] }), // vmax{x,y,z,w}
            0x14..=0x17 => lanes(&|i| if a[i] < bc[i] { a[i] } else { bc[i] }), // vmini{x,y,z,w}
            0x1c => lanes(&|i| vu::mul(a[i], q)),
            0x20 => lanes(&|i| vu::add(a[i], q)),
            0x21 => lanes(&|i| vu::madd(acc[i], a[i], q)),
            0x24 => lanes(&|i| vu::sub(a[i], q)),
            0x25 => lanes(&|i| vu::msub(acc[i], a[i], q)),
            0x2b => lanes(&|i| if a[i] > b[i] { a[i] } else { b[i] }), // vmax
            0x2f => lanes(&|i| if a[i] < b[i] { a[i] } else { b[i] }), // vmini
            0x28 => lanes(&|i| vu::add(a[i], b[i])),
            0x29 => lanes(&|i| vu::madd(acc[i], a[i], b[i])),
            0x2a => lanes(&|i| vu::mul(a[i], b[i])),
            0x2c => lanes(&|i| vu::sub(a[i], b[i])),
            0x2d => lanes(&|i| vu::msub(acc[i], a[i], b[i])),
            0x2e => lanes(&|i| vu::msub(acc[i], a[cy[i]], b[cz[i]])), // vopmsub
            0x38 | 0x39 => {
                // vcallms (the start address is in the instruction, in 8-byte units) / vcallmsr (CMSAR0)
                let start = if fun == 0x38 { w >> 6 & 0x7fff } else { self.cmsar0 };
                self.vu0_run(start, 1 << 20);
                return;
            }
            0x3c..=0x3f => {
                // the "special" group: accumulator targets, moves, and the Q unit
                let op = (w >> 6 & 31) << 2 | (w & 3);
                let ftf = (w >> 23 & 3) as usize;
                let to_acc = match op {
                    0..=3 => Some(lanes(&|i| vu::add(a[i], bc[i]))),
                    4..=7 => Some(lanes(&|i| vu::sub(a[i], bc[i]))),
                    8..=11 => Some(lanes(&|i| vu::madd(acc[i], a[i], bc[i]))),
                    12..=15 => Some(lanes(&|i| vu::msub(acc[i], a[i], bc[i]))),
                    0x18..=0x1b => Some(lanes(&|i| vu::mul(a[i], bc[i]))),
                    0x10..=0x17 => {
                        // vitof0/4/12/15 and vftoi0/4/12/15: fixed point with 0, 4, 12 or 15 fraction bits
                        let scale = [1.0f32, 16.0, 4096.0, 32768.0][(op & 3) as usize];
                        let v = if op < 0x14 {
                            lanes(&|i| a[i].to_bits() as i32 as f32 / scale)
                        } else {
                            lanes(&|i| f32::from_bits((a[i] * scale) as i32 as u32))
                        };
                        self.put_vf(ft, dest, v);
                        None
                    }
                    0x1c => Some(lanes(&|i| vu::mul(a[i], q))),                  // vmulaq
                    0x1d => {
                        self.put_vf(ft, dest, lanes(&|i| a[i].abs()));           // vabs
                        None
                    }
                    0x20 => Some(lanes(&|i| vu::add(a[i], q))),                  // vaddaq
                    0x21 => Some(lanes(&|i| vu::madd(acc[i], a[i], q))),         // vmaddaq
                    0x24 => Some(lanes(&|i| vu::sub(a[i], q))),                  // vsubaq
                    0x25 => Some(lanes(&|i| vu::msub(acc[i], a[i], q))),         // vmsubaq
                    0x28 => Some(lanes(&|i| vu::add(a[i], b[i]))),
                    0x29 => Some(lanes(&|i| vu::madd(acc[i], a[i], b[i]))),
                    0x2a => Some(lanes(&|i| vu::mul(a[i], b[i]))),
                    0x2c => Some(lanes(&|i| vu::sub(a[i], b[i]))),
                    0x2e => Some(lanes(&|i| vu::mul(a[cy[i]], b[cz[i]]))), // vopmula
                    0x2f | 0x3b => None, // vnop, vwaitq (nothing to wait for without pipelines)
                    0x30 => {
                        self.put_vf(ft, dest, a); // vmove
                        None
                    }
                    0x31 => {
                        self.put_vf(ft, dest, [a[1], a[2], a[3], a[0]]); // vmr32
                        None
                    }
                    0x38 => {
                        self.q = vu::div(a[(w >> 21 & 3) as usize], b[ftf]);
                        None
                    }
                    0x39 => {
                        self.q = vu::sqrt(b[ftf]);
                        None
                    }
                    0x3a => {
                        self.q = vu::rsqrt(a[(w >> 21 & 3) as usize], b[ftf]);
                        None
                    }
                    _ => self.unknown(w, here, "VU0 macro special op"),
                };
                if let Some(v) = to_acc {
                    for (i, &lane) in v.iter().enumerate() {
                        if dest >> (3 - i) & 1 == 1 {
                            self.vacc[i] = lane; // the accumulator takes only the dest lanes
                        }
                    }
                }
                return;
            }
            _ => self.unknown(w, here, "VU0 macro op"),
        };
        self.put_vf(fdn, dest, out);
    }

    /// MMI: the EE's own 128-bit integer group. Only the parts a compiler emits for ordinary code are
    /// here — the multiply-adds, the second multiplier, the quadword moves and the bitwise ops.
    fn mmi(&mut self, w: u32, here: u32) {
        let (rs, rt, rd, sa, fun) = ((w >> 21 & 31) as usize, (w >> 16 & 31) as usize,
                                     (w >> 11 & 31) as usize, w >> 6 & 31, w & 63);
        let (s, t) = (self.r(rs), self.r(rt));
        let (qs, qt) = (self.gpr[rs], self.gpr[rt]);
        let q = |e: &mut Self, v: u128| {
            if rd != 0 {
                e.gpr[rd] = v;
            }
        };
        let lo64 = u64::MAX as u128;
        match (fun, sa) {
            (0x00, _) | (0x01, _) => {
                // madd / maddu: {hi, lo} += rs × rt (32-bit), and rd takes the new lo
                let acc = (self.hi as u32 as u64) << 32 | self.lo as u32 as u64;
                let p = if fun == 0 {
                    (s as i32 as i64 * t as i32 as i64) as u64
                } else {
                    (s as u32 as u64) * (t as u32 as u64)
                };
                let v = acc.wrapping_add(p);
                (self.lo, self.hi) = (sx32(v as u32), sx32((v >> 32) as u32));
                self.set(rd, self.lo);
            }
            (0x10, _) => self.set(rd, self.hi1),
            (0x11, _) => self.hi1 = s,
            (0x12, _) => self.set(rd, self.lo1),
            (0x13, _) => self.lo1 = s,
            (0x18, _) | (0x19, _) => {
                let p = if fun == 0x18 {
                    (s as i32 as i64 * t as i32 as i64) as u64
                } else {
                    (s as u32 as u64) * (t as u32 as u64)
                };
                (self.lo1, self.hi1) = (sx32(p as u32), sx32((p >> 32) as u32));
                self.set(rd, self.lo1);
            }
            (0x1a, _) => (self.lo1, self.hi1) = idiv(s as u32, t as u32),
            (0x1b, _) => (self.lo1, self.hi1) = udiv(s as u32, t as u32),
            (0x09, 0x0e) => q(self, (qt & lo64) | (qs & lo64) << 64), // pcpyld
            (0x09, 0x12) => q(self, qs & qt),                         // pand
            (0x09, 0x13) => q(self, qs ^ qt),                         // pxor
            (0x29, 0x0e) => q(self, (qs >> 64) | (qt >> 64) << 64),   // pcpyud
            (0x29, 0x12) => q(self, qs | qt),                         // por
            (0x29, 0x13) => q(self, !(qs | qt)),                      // pnor
            // Lane arithmetic, packs, extends and shuffles (behave: hand-written VU0 code uses them).
            (0x08, 0x00) => q(self, lanes(qs, qt, 32, |a, b| a.wrapping_add(b))),          // paddw
            (0x08, 0x01) => q(self, lanes(qs, qt, 32, |a, b| a.wrapping_sub(b))),          // psubw
            (0x08, 0x02) => q(self, lanes(qs, qt, 32, |a, b| mask(32, sxl(a, 32) > sxl(b, 32)))), // pcgtw
            (0x08, 0x03) => q(self, lanes(qs, qt, 32, |a, b| if sxl(a, 32) > sxl(b, 32) { a } else { b })), // pmaxw
            (0x08, 0x04) => q(self, lanes(qs, qt, 16, |a, b| a.wrapping_add(b))),          // paddh
            (0x08, 0x05) => q(self, lanes(qs, qt, 16, |a, b| a.wrapping_sub(b))),          // psubh
            (0x08, 0x06) => q(self, lanes(qs, qt, 16, |a, b| mask(16, sxl(a, 16) > sxl(b, 16)))), // pcgth
            (0x08, 0x07) => q(self, lanes(qs, qt, 16, |a, b| if sxl(a, 16) > sxl(b, 16) { a } else { b })), // pmaxh
            (0x08, 0x08) => q(self, lanes(qs, qt, 8, |a, b| a.wrapping_add(b))),           // paddb
            (0x08, 0x09) => q(self, lanes(qs, qt, 8, |a, b| a.wrapping_sub(b))),           // psubb
            (0x08, 0x0a) => q(self, lanes(qs, qt, 8, |a, b| mask(8, sxl(a, 8) > sxl(b, 8)))), // pcgtb
            (0x08, 0x10) => q(self, lanes(qs, qt, 32, |a, b| sat(sxl(a, 32) + sxl(b, 32), 32))),   // paddsw
            (0x08, 0x11) => q(self, lanes(qs, qt, 32, |a, b| sat(sxl(a, 32) - sxl(b, 32), 32))),   // psubsw
            (0x08, 0x14) => q(self, lanes(qs, qt, 16, |a, b| sat(sxl(a, 16) + sxl(b, 16), 16))),   // paddsh
            (0x08, 0x15) => q(self, lanes(qs, qt, 16, |a, b| sat(sxl(a, 16) - sxl(b, 16), 16))),   // psubsh
            (0x08, 0x18) => q(self, lanes(qs, qt, 8, |a, b| sat(sxl(a, 8) + sxl(b, 8), 8))),       // paddsb
            (0x08, 0x19) => q(self, lanes(qs, qt, 8, |a, b| sat(sxl(a, 8) - sxl(b, 8), 8))),       // psubsb
            (0x28, 0x10) => q(self, lanes(qs, qt, 32, |a, b| (a + b).min(0xffff_ffff))),           // padduw
            (0x28, 0x11) => q(self, lanes(qs, qt, 32, |a, b| a.saturating_sub(b))),                // psubuw
            (0x28, 0x14) => q(self, lanes(qs, qt, 16, |a, b| (a + b).min(0xffff))),                // padduh
            (0x28, 0x15) => q(self, lanes(qs, qt, 16, |a, b| a.saturating_sub(b))),                // psubuh
            (0x28, 0x18) => q(self, lanes(qs, qt, 8, |a, b| (a + b).min(0xff))),                   // paddub
            (0x28, 0x19) => q(self, lanes(qs, qt, 8, |a, b| a.saturating_sub(b))),                 // psubub
            (0x08, 0x12) => q(self, interleave(qs, qt, 32, 0)),                             // pextlw
            (0x08, 0x13) => q(self, pack(qs, qt, 32)),                                      // ppacw
            (0x08, 0x16) => q(self, interleave(qs, qt, 16, 0)),                             // pextlh
            (0x08, 0x17) => q(self, pack(qs, qt, 16)),                                      // ppach
            (0x08, 0x1a) => q(self, interleave(qs, qt, 8, 0)),                              // pextlb
            (0x08, 0x1b) => q(self, pack(qs, qt, 8)),                                       // ppacb
            (0x28, 0x01) => q(self, lanes(0, qt, 32, |_, b| (sxl(b, 32).unsigned_abs() as u128).min(0x7fff_ffff))), // pabsw
            (0x28, 0x02) => q(self, lanes(qs, qt, 32, |a, b| mask(32, a == b))),           // pceqw
            (0x28, 0x03) => q(self, lanes(qs, qt, 32, |a, b| if sxl(a, 32) < sxl(b, 32) { a } else { b })), // pminw
            (0x28, 0x05) => q(self, lanes(0, qt, 16, |_, b| (sxl(b, 16).unsigned_abs() as u128).min(0x7fff))), // pabsh
            (0x28, 0x06) => q(self, lanes(qs, qt, 16, |a, b| mask(16, a == b))),           // pceqh
            (0x28, 0x07) => q(self, lanes(qs, qt, 16, |a, b| if sxl(a, 16) < sxl(b, 16) { a } else { b })), // pminh
            (0x28, 0x0a) => q(self, lanes(qs, qt, 8, |a, b| mask(8, a == b))),             // pceqb
            (0x28, 0x12) => q(self, interleave(qs, qt, 32, 64)),                            // pextuw
            (0x28, 0x16) => q(self, interleave(qs, qt, 16, 64)),                            // pextuh
            (0x28, 0x1a) => q(self, interleave(qs, qt, 8, 64)),                             // pextub
            (0x09, 0x1a) => q(self, shuffle(qt, 16, [2, 1, 0, 3, 6, 5, 4, 7])),            // pexeh
            (0x09, 0x1b) => q(self, shuffle(qt, 16, [3, 2, 1, 0, 7, 6, 5, 4])),            // prevh
            (0x09, 0x1e) => q(self, shuffle(qt, 32, [2, 1, 0, 3])),                         // pexew
            (0x09, 0x1f) => q(self, shuffle(qt, 32, [1, 2, 0, 3])),                         // prot3w
            (0x29, 0x0a) => q(self, (0..4).fold(0, |v, k| v | (qt >> (32 * k) & 0xffff) << (32 * k)
                                                       | (qs >> (32 * k) & 0xffff) << (32 * k + 16))), // pinteh
            (0x29, 0x1a) => q(self, shuffle(qt, 16, [0, 2, 1, 3, 4, 6, 5, 7])),            // pexch
            (0x29, 0x1b) => q(self, (0..8).fold(0, |v, k| v | (qt >> if k < 4 { 0 } else { 64 } & 0xffff) << (16 * k))), // pcpyh
            (0x29, 0x1e) => q(self, shuffle(qt, 32, [0, 2, 1, 3])),                         // pexcw
            (0x34, _) | (0x36, _) | (0x37, _) => q(self, lanes(0, qt, 16, |_, b| match fun { // psllh, psrlh, psrah
                0x34 => b << (sa & 15) & 0xffff,
                0x36 => b >> (sa & 15),
                _ => (sxl(b, 16) >> (sa & 15)) as u128 & 0xffff,
            })),
            (0x3c, _) | (0x3e, _) | (0x3f, _) => q(self, lanes(0, qt, 32, |_, b| match fun { // psllw, psrlw, psraw
                0x3c => b << sa & 0xffff_ffff,
                0x3e => b >> sa,
                _ => (sxl(b, 32) >> sa) as u128 & 0xffff_ffff,
            })),
            _ => self.unknown(w, here, "MMI op"),
        }
    }
}

/// A lane of `bits` bits, sign-extended.
fn sxl(v: u128, bits: u32) -> i128 {
    ((v << (128 - bits)) as i128) >> (128 - bits)
}

/// A signed sum clamped to a `bits`-bit lane.
fn sat(v: i128, bits: u32) -> u128 {
    let (lo, hi) = (-(1i128 << (bits - 1)), (1i128 << (bits - 1)) - 1);
    v.clamp(lo, hi) as u128 & ((1u128 << bits) - 1)
}

/// All ones in a lane when `c` holds.
fn mask(bits: u32, c: bool) -> u128 {
    if c { (1u128 << bits) - 1 } else { 0 }
}

/// `f` on each pair of `bits`-bit lanes of `a` and `b`.
fn lanes(a: u128, b: u128, bits: u32, f: impl Fn(u128, u128) -> u128) -> u128 {
    let m = (1u128 << bits) - 1;
    (0..128 / bits).fold(0, |v, k| v | (f(a >> (bits * k) & m, b >> (bits * k) & m) & m) << (bits * k))
}

/// pext{l,u}{w,h,b}: the low (or high, `from` 64) halves' lanes of `t` and `s`, alternately, `t`'s first.
fn interleave(s: u128, t: u128, bits: u32, from: u32) -> u128 {
    let m = (1u128 << bits) - 1;
    (0..64 / bits).fold(0, |v, k| {
        v | (t >> (from + bits * k) & m) << (2 * bits * k) | (s >> (from + bits * k) & m) << (2 * bits * k + bits)
    })
}

/// ppac{w,h,b}: the even lanes of `t` in the low half, those of `s` in the high half.
fn pack(s: u128, t: u128, bits: u32) -> u128 {
    let m = (1u128 << bits) - 1;
    let n = 64 / bits;
    (0..n).fold(0, |v, k| v | (t >> (2 * bits * k) & m) << (bits * k) | (s >> (2 * bits * k) & m) << (64 + bits * k))
}

/// Lane `k` of the result is lane `order[k]` of `v`.
fn shuffle<const N: usize>(v: u128, bits: u32, order: [u32; N]) -> u128 {
    let m = (1u128 << bits) - 1;
    order.iter().enumerate().fold(0, |r, (k, &o)| r | (v >> (bits * o) & m) << (bits * k as u32))
}

/// A one-line summary. Deliberately *not* derived: a derived `Debug` on this struct would print all of
/// RAM, and a failing `assert_eq!` would take the terminal (and an agent's context) with it.
impl std::fmt::Debug for Ee {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "Ee {{ pc {:#010x}, v0 {:#x}, f0 {}, {} steps, {} MB RAM, {} byte(s) written, {} hook(s) }}",
               self.pc, self.v0(), self.f0(), self.steps, self.ram.len() >> 20, self.written.len(),
               self.hooks.len())
    }
}

fn sx32(v: u32) -> u64 {
    v as i32 as i64 as u64
}

/// `div`: the R5900 does not trap, it defines the degenerate cases.
fn idiv(s: u32, t: u32) -> (u64, u64) {
    let (s, t) = (s as i32, t as i32);
    match t {
        0 => (if s < 0 { 1 } else { u64::MAX }, s as i64 as u64),
        -1 if s == i32::MIN => (sx32(s as u32), 0),
        _ => (sx32((s / t) as u32), sx32((s % t) as u32)),
    }
}

fn udiv(s: u32, t: u32) -> (u64, u64) {
    match s.checked_div(t) {
        None => (u64::MAX, sx32(s)),
        Some(q) => (sx32(q), sx32(s % t)),
    }
}
