//! The function oracle: run one of the original game's own functions, on inputs you choose, with no
//! emulator anywhere.
//!
//! # What it is for
//!
//! A port is "probably right" until something proves it. The usual proof is a recording: drive the real
//! game under an emulator, log what a function produced, compare. That works, but a recording only ever
//! contains the inputs the game happened to produce — a few hundred, all from normal play, none of them
//! near the edges where ports actually break.
//!
//! This crate loads the game's code into a small R5900 interpreter and calls the function directly. A test
//! can then do what a recording cannot:
//!
//! ```no_run
//! use ps2_oracle::{Disc, Ee};
//! let mut disc = Disc::iso("game.iso".as_ref()).expect("your own disc");
//! disc.check_serial("SLUS-20000").expect("the build the addresses belong to");
//! let (boot, _) = disc.boot2().unwrap();
//! let mut ee = Ee::from_program(32 << 20, &disc.read(&boot).unwrap(), &[]).unwrap();
//! let f = addr("angle_between");                 // from your notes, never hard-coded in a crate
//! for k in 0..10_000u32 {
//!     let (x, y) = (rnd(k), rnd(k ^ 0x9e37));
//!     ee.call(f, &[], &[x, y], 100_000);
//!     assert_eq!(ee.f0().to_bits(), my_port::angle_between(x, y).to_bits(),
//!                "angle_between({x}, {y})");   // first mismatch names its inputs
//! }
//! # fn rnd(_: u32) -> f32 { 0.0 }
//! # fn addr(_: &str) -> u32 { 0 }
//! # mod my_port { pub fn angle_between(_: f32, _: f32) -> f32 { 0.0 } }
//! ```
//!
//! Ten thousand cases take a second or two. Every one of them is bit-exact or the test fails with the
//! inputs that broke it, which is the whole point: you find the disagreement, not the next player.
//!
//! # What it can prove
//!
//! A pure function of its arguments and of RAM. In practice that is most of what a port gets wrong:
//! maths and geometry, table lookups and interpolation, AI decisions, state machines whose state you can
//! load from a RAM dump, scoring and rules, random number generators, string and data decoders.
//!
//! # What it cannot prove, at all
//!
//! Anything whose answer depends on *when* it ran, or on hardware this interpreter does not contain:
//!
//! * **Timing and frame flow** — how many frames a state lasts, what order subsystems update in, what a
//!   function returns on the second call because something else moved in between.
//! * **DMA** — transfers complete instantly or not at all; a function that waits on one will spin until
//!   the instruction budget runs out. Hook it. The one exception is VIF0: [`Ee::vif0_dma`] replays a DMA
//!   chain of VIF codes, which is how a game uploads its VU0 micro programs at boot.
//! * **VU1** — nothing of it. VU0 in micro mode *is* run ([`Ee::vu0_run`], started by `vcallms`), with the
//!   pipeline timing a program's results can depend on; see [`vu0`] for what that model leaves out.
//! * **The GS** — nothing is drawn, no register has an effect, and no amount of calling will tell you what
//!   a frame looked like.
//! * **The IOP, the SPU, the BIOS and the kernel** — no syscalls, no threads, no files, no sound, no pad.
//! * **Memory it was never given** — an uninitialised global reads as zero rather than as whatever a
//!   running game had there. That is the commonest cause of a false mismatch; see [`Ee::from_ram_file`].
//!
//! For all of that, record the real machine instead (an emulator over PINE). The two techniques are
//! complementary, and the division is sharp: the oracle proves *what* a function computes, a recording
//! proves *when* it is called.
//!
//! # Contracts worth knowing before you start
//!
//! **Unknown instructions panic, with the address and the encoded word.** The interpreter implements what
//! a C compiler emits plus the EE's common extras; it is not complete, and it does not pretend to be. The
//! message names the instruction and where it was. The fix is to add the instruction to `src/ee.rs` —
//! usually a dozen lines next to its neighbours, with a test in `tests/interpret.rs` — not to work around
//! it.
//!
//! **This crate knows no game.** It takes bytes, paths and load addresses. Which disc, which overlays, and
//! the function addresses belong to the project using it, kept in a file of `name 0xaddr` lines rather than
//! in source: addresses are specific to one build of one game, and the name is the documentation.
//!
//! **Check the serial.** [`Disc::check_serial`] refuses a disc whose `SYSTEM.CNF` boot serial is not the
//! one your addresses came from. An address from a US build means something else entirely in the European
//! one, and silent nonsense is worse than an error.
//!
//! **Floats go through [`ps2_float`].** The interpreter's arithmetic *is* the model the port under test
//! uses, so a mismatch is always a difference in logic, never a difference in rounding.
//!
//! # Roughly what the interpreter knows
//!
//! * The integer set a C compiler emits: arithmetic and logic (32- and 64-bit), shifts, the branches and
//!   their delay slots including the "likely" forms, `mult`/`div` with the R5900's defined degenerate
//!   cases, `madd`/`maddu`, the second multiplier (`mult1`, `mfhi1`, …), loads and stores of every width, the unaligned
//!   pairs (`lwl`/`lwr`/`ldl`/`ldr` and their stores), and quadwords (`lq`/`sq`).
//! * COP1 single precision in full, through [`ps2_float::fpu`]: the accumulator forms (`adda.s`,
//!   `madd.s`, …), `rsqrt.s`, `max.s`/`min.s`, the compares and `bc1t`/`bc1f`, `cvt.w.s`/`cvt.s.w`.
//! * VU0 macro mode through [`ps2_float::vu`]: the lane-masked and broadcast add/sub/mul/madd/msub,
//!   the `Q` unit (`vdiv`, `vsqrt`, `vrsqrt`), `vopmula`/`vopmsub`, `vmove`/`vmr32`, `qmfc2`/`qmtc2`,
//!   `lqc2`/`sqc2`, `cfc2`/`ctc2`.
//! * VU0 micro mode ([`vu0`]): `vcallms`/`vcallmsr` run a program from VU0's micro memory to its end bit —
//!   the FMAC and divider ops, loads and stores, integer ops and branches, flag reads — with FMAC stalls,
//!   the `Q` latency and the flag latency modelled as the reference emulator does.
//! * The MMI instructions that turn up in ordinary compiled code: `pcpyld`/`pcpyud`, `pand`/`por`/
//!   `pxor`/`pnor`, and the `*1` multiplier moves.
//!
//! Everything else — COP0, VU1, the cache instructions' effects, syscalls — panics by name.
//! Teaching it one more instruction is a dozen lines in `src/ee.rs`.
//!
//! # Getting state in
//!
//! Three ways, in increasing order of effort:
//!
//! 1. [`Ee::from_program`] — code from the disc ([`Disc`], [`disc::elf_segments`]), RAM otherwise zero.
//!    Enough for anything that only reads its arguments and constant tables (which, in a shipped game, is
//!    most maths).
//! 2. [`Ee::from_ram_file`] / [`Ee::with_ram`] — a RAM image saved from a running game, so globals, loaded
//!    data and object pools are all exactly as they were at that moment. No disc needed, and the fastest
//!    way to make a stubborn function behave; [`Ee::holds`] checks it came from your build.
//! 3. Build the state yourself: [`Ee::write`], [`Ee::set_f32`] and friends, with [`Ee::scratch`] for
//!    out-parameters. Verbose, but it is the only way to reach a case the game never reached.

#![forbid(unsafe_code)]

pub mod disc;
pub mod ee;
pub mod vif;
pub mod vu0;

pub use disc::Disc;
pub use ee::{DONE, Ee, Hook};
