//! behave: does a candidate do what the retail function does?
//!
//! tools/equiv.py compares the two functions' operations, which cannot follow retail's VU0 code against a
//! candidate's plain float C. This runs both instead, in ps2-oracle's R5900 interpreter (vendored, MIT), on
//! the same random state many times over, and compares what each did:
//!
//! - every byte either wrote outside its own stack frame, at the end;
//! - every call out of the function, in order: where to, and the arguments the callee takes (a pointer into
//!   the caller's frame is compared by the 64 bytes it points at, not by its address);
//! - the result, by the candidate's return type.
//!
//! `--data` makes the program's initialised data random as well (tables, lists and flags the function reads):
//! more of the function is reached, from states the game may never have. `--tame` leaves raw random words out,
//! `--uncovered` lists retail's instructions no trial reached.
//!
//! A call is anything that leaves the function's bytes. It is recorded and returns a value made from the call's
//! position and target, the same in both runs. A float that differs only in its last bits (VU0 and the FPU round
//! differently) counts as close, not different. A trial in which retail itself faults (a pointer the random
//! state made up, a loop that does not end) is skipped.
//!
//! The manifest is written by `tools/equiv.py --behave`: the program (`elf`, `load`), the regions to
//! randomise (`fill`), `$gp` (`gp`), the function (`func ADDRESS RETAIL_SIZE`), the candidate's relocated bytes (`cand`), its
//! return type (`ret void|int|float|long`), its integer arguments (`params p i ...`: pointer or number) and the
//! callees' arguments (`arity ADDRESS INTS FLOATS`).
//!
//!   behave MANIFEST [--trials N] [--seed S] [--show N] [--tame] [--data] [--uncovered]

use std::cell::Cell;
use std::collections::{BTreeMap, BTreeSet};
use std::panic::{AssertUnwindSafe, catch_unwind};
use std::path::PathBuf;

use ps2_oracle::{DONE, Ee};

/// The console's 32 MB, and room above it for the arena and the stack.
const RAM: usize = 34 << 20;
/// Random structures live here, above the console's 32 MB, where no program address points.
const ARENA: u32 = 0x0200_0000;
const SLOTS: u32 = 256;
const SLOT: u32 = 0x1000;
const ARENA_END: u32 = ARENA + SLOTS * SLOT + 0x1_0000;
const STACK: u32 = (RAM as u32) - 0x4000;
/// The callee's frame: below the stack pointer a call starts with.
const FRAME: u32 = 0x1_0000;
const BUDGET: u64 = 400_000;

#[derive(Clone, Copy, PartialEq)]
enum Ret {
    Void,
    Int,
    Long,
    Float,
}

struct Job {
    elf: PathBuf,
    loads: Vec<(u32, PathBuf)>,
    fills: Vec<(u32, u32)>,
    /// Initialised data, made random too with `--data`.
    data: Vec<(u32, u32)>,
    func: u32,
    rsize: u32,
    gp: u32,
    cand: PathBuf,
    ret: Ret,
    arity: BTreeMap<u32, (usize, usize)>,
    /// The candidate's integer arguments, `p` for a pointer and `i` for a number; None when unknown.
    params: Option<Vec<char>>,
}

fn num(s: &str) -> u32 {
    let s = s.trim();
    if let Some(h) = s.strip_prefix("0x") { u32::from_str_radix(h, 16) } else { s.parse() }
        .unwrap_or_else(|_| panic!("not a number: {s}"))
}

fn read_job(path: &str) -> Job {
    let text = std::fs::read_to_string(path).unwrap_or_else(|e| panic!("{path}: {e}"));
    let mut job = Job {
        elf: PathBuf::new(),
        loads: vec![],
        fills: vec![],
        data: vec![],
        func: 0,
        rsize: 0,
        gp: 0,
        cand: PathBuf::new(),
        ret: Ret::Void,
        arity: BTreeMap::new(),
        params: None,
    };
    for line in text.lines() {
        let w: Vec<&str> = line.split_whitespace().collect();
        match w.as_slice() {
            [] => {}
            [c, ..] if c.starts_with('#') => {}
            ["elf", p] => job.elf = p.into(),
            ["load", a, p] => job.loads.push((num(a), p.into())),
            ["fill", a, n] => job.fills.push((num(a), num(n))),
            ["data", a, n] => job.data.push((num(a), num(n))),
            ["func", a, n] => (job.func, job.rsize) = (num(a), num(n)),
            ["cand", p] => job.cand = p.into(),
            ["gp", a] => job.gp = num(a),
            ["ret", r] => {
                job.ret = match *r {
                    "void" => Ret::Void,
                    "float" => Ret::Float,
                    "long" => Ret::Long,
                    _ => Ret::Int,
                }
            }
            ["arity", a, i, f] => {
                job.arity.insert(num(a), (num(i) as usize, num(f) as usize));
            }
            ["params", ps @ ..] => job.params = Some(ps.iter().filter_map(|s| s.chars().next()).collect()),
            _ => panic!("{path}: cannot read `{line}`"),
        }
    }
    job
}

/// `--tame`: no words of random bits in the state, which make floats no game value has (1e30 as an angle).
static TAME: std::sync::atomic::AtomicBool = std::sync::atomic::AtomicBool::new(false);

/// Constants retail's code compares against and builds (`li`, `ori`, `andi`, `slti`, `lui`): some of the random
/// state takes them, so a test for one value (a state of -2, a flag bit, a float limit) is reached.
static POOL: std::sync::OnceLock<Vec<u32>> = std::sync::OnceLock::new();

fn constants(code: &[u8]) -> Vec<u32> {
    let mut out = BTreeSet::new();
    for w in code.chunks_exact(4).map(|c| u32::from_le_bytes(c.try_into().unwrap())) {
        let (op, rs, imm) = (w >> 26, w >> 21 & 31, w & 0xffff);
        let simm = imm as i16 as i32 as u32;
        match op {
            0x09 | 0x19 if rs == 0 => {
                out.insert(simm);
            }
            0x0d if rs == 0 => {
                out.insert(imm);
            }
            0x0c => {
                out.insert(imm);
                out.insert(!imm);
            }
            0x0a | 0x0b => {
                out.insert(simm);
                out.insert(simm.wrapping_sub(1));
            }
            0x0f => {
                out.insert(imm << 16);
            }
            _ => {}
        }
    }
    out.into_iter().collect()
}

/// xorshift64*: the same stream for both runs of a trial.
struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u64 {
        self.0 ^= self.0 >> 12;
        self.0 ^= self.0 << 25;
        self.0 ^= self.0 >> 27;
        self.0.wrapping_mul(0x2545_F491_4F6C_DD1D)
    }
    fn below(&mut self, n: u64) -> u64 {
        self.next() % n
    }
    fn pointer(&mut self) -> u32 {
        ARENA + self.below(SLOTS as u64) as u32 * SLOT
    }
    fn float(&mut self) -> f32 {
        match self.below(8) {
            0 => 0.0,
            1 => 1.0,
            2 => -1.0,
            3 => (self.below(2001) as f32 - 1000.0) * 0.5,
            _ => (self.below(20001) as f32 - 10000.0) / 100.0,
        }
    }
    /// What a word of state holds: a pointer to a structure, a small number, a float or anything.
    fn word(&mut self) -> u32 {
        match self.below(20) {
            0..=6 => self.pointer(),
            7..=9 => (self.below(12) as i32 - 3) as u32,
            10 | 11 => match POOL.get() {
                Some(p) if !p.is_empty() => p[self.below(p.len() as u64) as usize],
                _ => self.below(9) as u32,
            },
            12 => u32::MAX,
            13..=17 => self.float().to_bits(),
            _ if TAME.load(std::sync::atomic::Ordering::Relaxed) => self.float().to_bits(),
            _ => self.next() as u32,
        }
    }
    fn arg(&mut self) -> u64 {
        match self.below(4) {
            0 | 1 => self.pointer() as u64,
            2 => self.below(9),
            _ => self.word() as i32 as u64,
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
enum Arg {
    Value(u64),
    Frame(Vec<u8>),
    Float(u32),
}

#[derive(Clone, Debug)]
struct Call {
    target: u32,
    args: Vec<Arg>,
}

struct Run {
    calls: Vec<Call>,
    fault: Option<String>,
    pcs: BTreeSet<u32>,
}

fn in_frame(a: u64) -> bool {
    let a = a as u32;
    (STACK - FRAME..STACK + 0x4000).contains(&a)
}

/// The value a call returns: from its position and target, the same in both runs.
fn reply(k: usize, target: u32) -> (u64, f32) {
    let mut r = Rng((k as u64 + 1).wrapping_mul(0x9E37_79B9_7F4A_7C15) ^ target as u64);
    r.next();
    (r.arg(), r.float())
}

fn run(ee: &mut Ee, job: &Job, size: u32, ints: &[u64], floats: &[f32]) -> Run {
    let calls = std::cell::RefCell::new(Vec::new());
    let pcs = std::cell::RefCell::new(BTreeSet::new());
    let steps = Cell::new(0u64);
    let body = job.func..job.func + size;
    let result = catch_unwind(AssertUnwindSafe(|| {
        for (k, &v) in ints.iter().enumerate() {
            ee.set(4 + k, v);
        }
        for (k, &v) in floats.iter().enumerate() {
            ee.fpr[12 + k] = v.to_bits();
        }
        ee.set(28, job.gp as u64);
        ee.set(29, STACK as i32 as i64 as u64);
        ee.set(31, DONE as i32 as i64 as u64);
        ee.pc = job.func;
        ee.npc = job.func + 4;
        ee.written.clear();
        while ee.pc != DONE {
            if steps.get() >= BUDGET {
                panic!("still running after {BUDGET} instructions (pc {:#x})", ee.pc);
            }
            if steps.get() & 1023 == 0 && ee.written.len() > 1 << 20 {
                panic!("wrote more than 1 MB (pc {:#x})", ee.pc);
            }
            if !body.contains(&ee.pc) {
                let target = ee.pc;
                let (ni, nf) = job.arity.get(&target).copied().unwrap_or((8, 8));
                let mut args = Vec::new();
                for k in 0..ni {
                    let v = ee.r(4 + k);
                    args.push(if in_frame(v) {
                        Arg::Frame((0..64).map(|o| ee.u8_at(v as u32 + o)).collect())
                    } else {
                        Arg::Value(v as u32 as i32 as i64 as u64)
                    });
                }
                for k in 0..nf {
                    args.push(Arg::Float(ee.fpr[12 + k]));
                }
                let mut c = calls.borrow_mut();
                if c.len() >= 4096 {
                    panic!("made more than 4096 calls (pc {:#x})", ee.pc);
                }
                steps.set(steps.get() + 1);
                let (v0, f0) = reply(c.len(), target);
                c.push(Call { target, args });
                ee.set(2, v0);
                ee.set_f(0, f0);
                ee.pc = ee.r(31) as u32;
                ee.npc = ee.pc.wrapping_add(4);
                continue;
            }
            pcs.borrow_mut().insert(ee.pc);
            ee.step();
            steps.set(steps.get() + 1);
        }
    }));
    let fault = result.err().map(|e| {
        e.downcast_ref::<String>().cloned()
            .or_else(|| e.downcast_ref::<&str>().map(|s| s.to_string()))
            .unwrap_or_else(|| "a fault".into())
            .lines().next().unwrap_or("").to_string()
    });
    Run { calls: calls.into_inner(), fault, pcs: pcs.into_inner() }
}

/// Floats that differ only in the last bits: what VU0's and the FPU's rounding leave.
fn close(a: u32, b: u32) -> bool {
    a >> 31 == b >> 31 && (a as i64 - b as i64).abs() <= 4 && (a >> 23 & 0xFF) != 0
}

#[derive(Default)]
struct Diff {
    exact: bool,
    close: bool,
    notes: Vec<String>,
}

impl Diff {
    fn word(&mut self, what: String, a: u32, b: u32) {
        if a == b {
            return;
        }
        if close(a, b) {
            self.close = true;
        } else {
            self.exact = false;
            self.notes.push(format!("{what}: retail {a:#010x}, candidate {b:#010x}"));
        }
    }
}

fn compare(job: &Job, a: &Ee, ra: &Run, b: &Ee, rb: &Run) -> Diff {
    let mut d = Diff { exact: true, ..Default::default() };
    if let Some(f) = &rb.fault {
        d.exact = false;
        d.notes.push(format!("the candidate faults: {f}"));
        return d;
    }
    let mut words = BTreeSet::new();
    for &w in a.written.iter().chain(b.written.iter()) {
        if !(STACK - FRAME..STACK).contains(&w) {
            words.insert(w & !3);
        }
    }
    for w in words {
        let (x, y) = (a.u32_at(w), b.u32_at(w));
        d.word(format!("the word at {w:#x}"), x, y);
    }
    if ra.calls.len() != rb.calls.len() {
        d.exact = false;
        d.notes.push(format!("{} calls in retail, {} in the candidate", ra.calls.len(), rb.calls.len()));
    }
    for (k, (x, y)) in ra.calls.iter().zip(rb.calls.iter()).enumerate() {
        if x.target != y.target {
            d.exact = false;
            d.notes.push(format!("call {k}: retail calls {:#x}, the candidate {:#x}", x.target, y.target));
            break;
        }
        for (n, (p, q)) in x.args.iter().zip(y.args.iter()).enumerate() {
            match (p, q) {
                (Arg::Float(p), Arg::Float(q)) => d.word(format!("call {k} to {:#x}, float argument {n}", x.target), *p, *q),
                (Arg::Value(p), Arg::Value(q)) if p == q => {}
                (Arg::Frame(p), Arg::Frame(q)) => {
                    for o in (0..64).step_by(4) {
                        let w = |v: &Vec<u8>| u32::from_le_bytes([v[o], v[o + 1], v[o + 2], v[o + 3]]);
                        d.word(format!("call {k} to {:#x}, argument {n} (a local) +{o:#x}", x.target), w(p), w(q));
                    }
                }
                _ => {
                    d.exact = false;
                    d.notes.push(format!("call {k} to {:#x}, argument {n}: retail {p:x?}, candidate {q:x?}", x.target));
                }
            }
        }
    }
    match job.ret {
        Ret::Void => {}
        Ret::Int => d.word("the result".into(), a.r(2) as u32, b.r(2) as u32),
        Ret::Long => {
            if a.r(2) != b.r(2) {
                d.exact = false;
                d.notes.push(format!("the result: retail {:#x}, candidate {:#x}", a.r(2), b.r(2)));
            }
        }
        Ret::Float => d.word("the float result".into(), a.fpr[0], b.fpr[0]),
    }
    d
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 || args[1].starts_with("--") {
        eprintln!("usage: behave MANIFEST [--trials N] [--seed S] [--show N] [--tame] [--data] [--uncovered]");
        std::process::exit(2);
    }
    let opt = |name: &str, default: u64| {
        args.iter().position(|a| a == name).map(|i| args[i + 1].parse().unwrap()).unwrap_or(default)
    };
    let trials = opt("--trials", 3000);
    let seed = opt("--seed", 1);
    let show = opt("--show", 3) as usize;
    TAME.store(args.iter().any(|a| a == "--tame"), std::sync::atomic::Ordering::Relaxed);
    let job = read_job(&args[1]);

    let elf = std::fs::read(&job.elf).unwrap_or_else(|e| panic!("{}: {e}", job.elf.display()));
    let loads: Vec<(u32, Vec<u8>)> = job.loads.iter()
        .map(|(a, p)| (*a, std::fs::read(p).unwrap_or_else(|e| panic!("{}: {e}", p.display()))))
        .collect();
    let refs: Vec<(u32, &[u8])> = loads.iter().map(|(a, b)| (*a, b.as_slice())).collect();
    let cand = std::fs::read(&job.cand).unwrap_or_else(|e| panic!("{}: {e}", job.cand.display()));
    let mut a = Ee::from_program(RAM, &elf, &refs).unwrap();
    let retail_code: Vec<u8> = (0..job.rsize).map(|k| a.u8_at(job.func + k)).collect();
    POOL.set(constants(&retail_code)).ok();
    let mut b = Ee::from_program(RAM, &elf, &refs).unwrap();
    // Only the console's 32 MB above the kernel, the arena and the stack; aligned accesses only.
    let valid = vec![(0x10_0000, 0x200_0000), (ARENA, ARENA_END), (STACK - FRAME, STACK + 0x4000)];
    a.strict = valid.clone();
    b.strict = valid;
    b.load(job.func, &cand);
    let pristine_a = a.ram.clone();
    let pristine_b = b.ram.clone();

    // The executable's .bss, the level's, the arena and the caller's side of the stack: random each trial.
    let mut fills = job.fills.clone();
    let ph = u32::from_le_bytes(elf[0x1c..0x20].try_into().unwrap()) as usize;
    let phn = u16::from_le_bytes([elf[0x2c], elf[0x2d]]) as usize;
    for k in 0..phn {
        let p = ph + 32 * k;
        let f = |o: usize| u32::from_le_bytes(elf[p + o..p + o + 4].try_into().unwrap());
        if f(0) == 1 && f(20) > f(16) {
            fills.push((f(8) + f(16), f(20) - f(16)));
        }
    }
    if args.iter().any(|a| a == "--data") {
        fills.extend(job.data.iter().copied());
    }
    fills.push((ARENA, ARENA_END - ARENA));
    fills.push((STACK, 0x4000));
    std::panic::set_hook(Box::new(|_| {}));

    let (mut same, mut near, mut differ, mut skipped) = (0u64, 0u64, 0u64, 0u64);
    let mut faults: BTreeMap<String, u64> = BTreeMap::new();
    let mut shown = 0;
    let mut covered = BTreeSet::new();
    for t in 0..trials {
        let mut r = Rng(seed.wrapping_mul(0x2545_F491_4F6C_DD1D) ^ (t + 1).wrapping_mul(0x9E37_79B9_7F4A_7C15));
        r.next();
        for (ee, pristine) in [(&mut a, &pristine_a), (&mut b, &pristine_b)] {
            for &w in ee.written.iter() {
                let at = (w & 0x0fff_ffff) as usize;
                if w >> 28 != 7 && at < RAM {
                    ee.ram[at] = pristine[at];
                }
            }
        }
        let seed_state = r.0;
        for ee in [&mut a, &mut b] {
            let mut r = Rng(seed_state);
            for &(start, len) in &fills {
                for o in (0..len & !3).step_by(4) {
                    let at = ((start + o) & 0x0fff_ffff) as usize;
                    ee.ram[at..at + 4].copy_from_slice(&r.word().to_le_bytes());
                }
            }
            for k in 1..32 {
                ee.gpr[k] = (r.next() as u128) << 64 | r.word() as i32 as i64 as u64 as u128;
            }
            for k in 0..32 {
                ee.fpr[k] = r.float().to_bits();
            }
            for k in 1..32 {
                ee.vf[k] = [r.float(), r.float(), r.float(), r.float()];
            }
            ee.vacc = [r.float(), r.float(), r.float(), r.float()];
            ee.acc = r.float().to_bits();
        }
        for _ in 0..64 {
            r.next();
        }
        let ints: Vec<u64> = match &job.params {
            Some(ps) => (0..8).map(|k| match ps.get(k) {
                Some('p') => r.pointer() as u64,
                Some(_) => r.word() as i32 as u64,
                None => r.arg(),
            }).collect(),
            None => (0..8).map(|_| r.arg()).collect(),
        };
        let floats: Vec<f32> = (0..8).map(|_| r.float()).collect();
        let ra = run(&mut a, &job, job.rsize, &ints, &floats);
        if let Some(f) = &ra.fault {
            skipped += 1;
            let key = f.split(" (pc").next().unwrap_or(f).chars().take(80).collect::<String>();
            *faults.entry(key).or_default() += 1;
            continue;
        }
        covered.extend(ra.pcs.iter().copied());
        let rb = run(&mut b, &job, cand.len() as u32, &ints, &floats);
        let d = compare(&job, &a, &ra, &b, &rb);
        if d.exact && !d.close {
            same += 1;
        } else if d.exact {
            near += 1;
        } else {
            differ += 1;
            if shown < show {
                shown += 1;
                println!("trial {t}: a0..a3 {:x?}, f12..f15 {:?}", &ints[..4], &floats[..4]);
                for n in d.notes.iter().take(8) {
                    println!("    {n}");
                }
            }
        }
    }
    let ran = trials - skipped;
    let cover = 100.0 * covered.len() as f64 / (job.rsize / 4).max(1) as f64;
    println!("{ran} of {trials} trials ran ({skipped} skipped: retail faulted); retail's instructions reached: {cover:.0}%");
    if args.iter().any(|a| a == "--uncovered") {
        let mut runs: Vec<(u32, u32)> = Vec::new();
        for pc in (job.func..job.func + job.rsize).step_by(4).filter(|pc| !covered.contains(pc)) {
            match runs.last_mut() {
                Some(r) if r.1 == pc => r.1 = pc + 4,
                _ => runs.push((pc, pc + 4)),
            }
        }
        let list: Vec<String> = runs.iter().map(|(s, e)| format!("{s:#x}..{e:#x}")).collect();
        println!("    not reached: {}", if list.is_empty() { "nothing".into() } else { list.join(" ") });
    }
    for (f, n) in faults.iter().rev().take(3) {
        println!("    skipped {n}: {f}");
    }
    let verdict = if ran == 0 {
        "NOT RUN (retail faulted on every trial)".to_string()
    } else if differ > 0 {
        format!("DIFFERENT ({differ} of {ran} trials)")
    } else if near > 0 {
        format!("SAME UP TO FLOAT ROUNDING ({near} of {ran} trials differ in the last bits)")
    } else {
        format!("SAME ({same} trials)")
    };
    println!("behave: {verdict}");
}
