# What the original code relies on in the PS2's hardware

The games' code silently relies on behaviour of the console's processors that a PC does not
share: how the EE's FPU rounds, that its floats have no infinities, that registers are 128
bits wide, and so on. This page goes through a list of seventeen such behaviours, from notes
from a static recompilation of Ratchet: Deadlocked PAL (the same engine family; each item broke
something visible there), and says for each how OpenRAC's native port stands.

The port does not emulate the console ([DESIGN.md](DESIGN.md)): the decompiled C is compiled
for the PC, and the functions that still have no C are translated from their assembly into C
by hostgen ([asmc.py](../../port/tools/hostgen/asmc.py), with the instruction helpers in
[ee.h](../../port/runtime/include/openrac/ee.h) and
[ee_ops.h](../../port/runtime/include/openrac/ee_ops.h)). The kernel, the graphics library, the
IOP and the sound are replaced at their library calls
([port/game/common/lib](../../port/game/common/lib)), and the renderer draws what the game
builds with renderers of its own. So an item can matter in three places: the matched C the
compiler translates (hostgen's [lower.py](../../port/tools/hostgen/lower.py)), the assembly
hostgen translates, and the library replacements and renderer. Some items only concern an
implementation that runs the console's own programs, which the port never does.

Verdicts: **a**, it applied to the port and was fixed (the commit is named); **b**, it was
already handled (where); **c**, it does not apply (why). The tests named are in
[test_hostgen.py](../../port/tools/hostgen/test_hostgen.py) (`FromAssembly`) unless said
otherwise.

| # | Behaviour | Verdict | Where |
|---|---|---|---|
| 1 | FPU rounds toward zero; no denormals | b (rounding, `cec43c70`), a (denormals, `79868e57`) | `openrac_ee_float_mode`, [ee.cpp](../../port/runtime/ee.cpp), called by [main.cpp](../../port/game/common/main.cpp) `run_game_as_the_ee` |
| 2 | No Inf/NaN; DIV by zero gives ±MAX | b, in part (`23cecd12`, `4afc86af`) | `openrac_fdiv` ([guest.h](../../port/runtime/include/openrac/guest.h)), `openrac_ee_sqrt`/`rsqrt` ([ee.h](../../port/runtime/include/openrac/ee.h)) |
| 3 | 128-bit registers, 64-bit branches, syscalls keep v0/v1's upper halves | b, and a (conditional calls, `d356c986`; the matched C's integer widths) | asmc.py `control`, `_invoke`; prep.py `widen_long_long`, `name_wide_casts`; lower.py `_as_param`; program.py `widen_forwarded` |
| 4 | vf0 is read only | b | asmc.py `vf`, `store`, `qmtc2`, `lqc2` |
| 5 | VFTOI and CVT.W.S saturate | a (exponent-255 values, `79868e57`) | `openrac_ee_cvtws` (ee.h), `openrac_ee_ftoi` (ee_ops.h) |
| 6 | VCALLMSR uses CMSAR0 | c | no microprogram runs |
| 7 | INTC_STAT is write-1-to-clear; SetVSyncFlag's stack variables | b (`03dff78c`) | `sceGsSyncV` replaced; `openrac_lib_SetVSyncFlag` ([kernel.c](../../port/game/common/lib/kernel.c)) |
| 8 | VU0 macro memory ops address VU0 data memory | a (`b9046220`) | asmc.py `vu0`, `openrac_vu0_addr` (ee_ops.h) |
| 9 | VCLIP, MAC and STATUS flags via CFC2 | a (`b9046220`) | asmc.py `vu0`, `openrac_vu0_mac`, `openrac_vu0_set_mac`, `openrac_vu0_set_div`, `openrac_vu0_clip` (ee_ops.h) |
| 10 | MMI semantics | b (most), a (the rest, `5b7f0765`) | `openrac_ee_mmi`, `openrac_ee_pmul`, `openrac_ee_pmfhl_*` (ee_ops.h); asmc.py `stmt` |
| 11 | VIF UNPACK V4-5 | b (sizes), c (contents) | `direct_packets` ([frontend.cpp](../../port/game/common/frontend.cpp)) |
| 12 | The GIF is a stream | b (`910fef95`) | `GifInterpreter::gif` ([direct.cpp](../../port/renderer/direct.cpp)), `direct_packets` |
| 13 | Progressive XGKICK | c | no VU1 program runs |
| 14 | Kernel thread priorities, no time-slicing | c | one thread: [kernel.c](../../port/game/common/lib/kernel.c) |
| 15 | SetupHeap with an explicit size | b | `openrac_lib_EndOfHeap` (kernel.c), rac4's `rac4EndOfHeap` |
| 16 | PAL timing at 50 Hz; lost frames not caught up | b | `FramePacer` ([clock.cpp](../../port/platform/clock.cpp)) |
| 17 | IOP RPC handlers that block | c | no IOP: `sceSifCallRpc` and the sound are host code |

## 1. The FPU rounds toward zero, and has no denormals

**Rounding: b.** The game's thread has run rounding toward zero since
`cec43c70` (`fix(port): run the game's code rounding toward zero, as the EE does`): a counter the
game steps by 0.1 crossed zero otherwise. The animation code's trick of adding and subtracting
12582912.0 to drop a fraction works the same way. Constants the compiler folds stay rounded to
nearest, which is right: the console's compiler folded them the same way when it built the
retail program, so the build does not take `-frounding-math`.

**Denormals: a.** The host kept denormals: a result the EE would have made zero stayed a tiny
number, and a denormal read from data was not zero. Since `79868e57` the same call that
sets the rounding also sets flush-to-zero and denormals-are-zero (MXCSR's FTZ and DAZ on x86,
FPCR.FZ on AArch64), for the game's thread only. `OPENRAC_DENORMALS=1` keeps the host's
behaviour, as `OPENRAC_ROUND_NEAREST=1` does for the rounding. Test: `runtime_test`
(`ee_float_mode`: 1.5 plus and minus 12582912 is 1, 1/3 truncates, denormals are zero).

## 2. No infinities or NaNs

**b, in part.** The game's float `/` becomes `openrac_fdiv`, which gives ±FLT_MAX with the XOR of
the signs for a zero (or denormal) divisor and saturates an overflowing quotient (`23cecd12`);
`div.s` and VU0's `vdiv` in translated assembly use it too. `sqrt.s`, `rsqrt.s`, `vsqrt` and
`vrsqrt` take the root of the magnitude (`4afc86af`; the camera snaps of 2026-10-11 were a root of
a negative number). `cvt.w.s` and `vftoi` saturate (item 5).

Not modelled: an operand with exponent 255 in an addition or a multiplication, and a sum or a
product that overflows. The host computes those in IEEE arithmetic, so a value such as
`0x7FFFEF02` is a NaN that spreads where the VU would have read a large number. Clamping every
operand and every result would cost each float operation of the game a compare or two; the
port fixes such places where they show (`OPENRAC_WATCH`, docs/port/DEBUGGING.md) rather than
everywhere. The MAC flags of item 9 do report an exponent-255 result as an overflow.

Where a function needs all of it, it can have it: a function named in its game's hostgen.json
`"ps2_float"` computes its float `+ - * /`, comparisons and conversions through
[ps2float.h](../../port/runtime/include/openrac/ps2float.h), the model of
[ps2_float.h](../../port/common/ps2_float.h) in software: an overflow is ±`0x7FFFFFFF`, an
exponent of 255 an ordinary number, and the adder drops the smaller operand's bits more than one
place below the larger one's last bit before it adds (where the host, even rounding toward zero,
cuts the exact sum: `1 - 1.5 * 2^-24` is `0x3F7FFFFF` on the console, `0x3F7FFFFE` on the host).
The port's own C calls the same functions. That is for functions whose result the game cuts to an
integer or compares, where one last place decides; the test is `ps2float` (ctest) and
`test_ps2_float_computes_as_the_ee_does`.

## 3. 128-bit registers, 64-bit branches, the upper halves across system calls

**b.** In translated assembly every general register is a 64-bit local, with a second 64-bit
half where the function uses `lq`, `sq` or MMI (asmc.py `R`, `H`). `blez`, `bgtz`, `bltz`, `bgez`
and their likely forms compare the whole 64-bit value (`(openrac_s64)rN < 0`, asmc.py
`control`), like `beq`, `bne`, `slt` and `sltu`. A system call never runs as such: the
function stays a stub, and the library replacements stand for the calls the game makes. A call
writes only the low 64 bits of `$v0` with its result (`_invoke`), so a hand-written routine that
keeps a value in the upper half of `$v0` or in `$v1` across a call keeps it. In the matched C,
`long` is the EE's 64-bit long (prep.py), and functions whose result fills all 64 bits of `$v0`
are listed in `returns_64` (`fdaaaa6b`).

**a: conditional calls.** `bltzal` and `bgezal` (and their likely forms) made the whole function
a stub ("a conditional call"; twelve functions in rac4). Since `d356c986` they are a call
when the 64-bit register's sign says so, with `$ra` written whether or not it calls; a callee
that takes its inputs in registers no prototype names runs in place, as a `jal` to one does.
Test: `test_conditional_call_tests_all_64_bits` (a register whose low word is positive and
whose doubleword is negative calls). rac4's twelve stay stubs for now: they share one
conditional call at `0x003F2CDC` whose delay slot is a jump, which hostgen does not write.

**a: the integer widths of the matched C.** The compiler that built rac1's game code (SN's GCC
2.95.3) has a 128-bit `long long`, and the EE hands a register on with all its 64 bits whatever
type the C gives it in between. hostgen reads the matched C so: a unit's `long long` is the
128-bit `openrac_ti` (hostgen.json `long_long_128`; the libraries Sony's compiler built keep a
64-bit one), and so is a `mode(TI)` cast on any integer type; a 32-bit value a call computes is
sign-extended where the callee takes 64 bits, as the EE's register holds it; a parameter the C
only passes on to 64-bit parameters is 64-bit itself (program.py `widen_forwarded`, report.json
`params_widened`); a `returns_64` result stays whole through a call by code address, and
through a definition with a 32-bit result that returns it as it is (program.py `widen_returns`,
report.json `results_widened`). An unsigned int or a pointer that an int local kept 64-bit for
such a result also receives is sign-extended, as `lui`/`ori` or `lw` leave it in the register.
A local the original keeps in a stack slot across calls holds only the sign-extended low half
after its `lw`; nothing in the C tells it apart from one kept in a saved register, so hostgen.json
`spilled_locals` names it (rac1-pal's freeze screen). Tests
(`Translate`, `Preparation`, `Places`): `test_a_long_long_copy_of_the_games_compiler_moves_a_quadword`,
`test_a_mode_ti_cast_on_any_integer_type_is_128_bit`,
`test_a_32_bit_argument_to_a_64_bit_parameter_is_sign_extended`,
`test_a_parameter_only_passed_on_to_64_bits_is_64_bit`,
`test_a_widened_parameter_is_whole_in_64_bit_casts_and_its_low_half_in_arithmetic`,
`test_a_copys_call_keeps_a_returns_64_result_whole`,
`test_a_returns_64_result_returned_as_it_is_is_64_bit_and_constants_beside_it_are_sign_extended`,
`test_an_unsigned_value_beside_a_returns_64_result_in_a_local_is_sign_extended`,
`test_a_spilled_local_keeps_a_returns_64_result_sign_extended`.

## 4. vf0 is read only

**b.** asmc.py reads `vf0` as the constants (0, 0, 0, 1) and never writes it: `store` skips
destination 0, `qmtc2` and `lqc2` to `vf0` do nothing, and so do the new loads of item 8. The
state's `vf[0]` is also initialised to (0, 0, 0, 1) ([ee.cpp](../../port/runtime/ee.cpp)).
`vi0` is likewise 0 and never written. Test: `test_vu0_macro_memory_and_vf0`.

## 5. VFTOI and CVT.W.S saturate

**b** for values in range: both truncate, and saturate to `0x7FFFFFFF` at 2^31 and above
(`openrac_ee_cvtws`, `openrac_ee_ftoi`); the matched C's conversions use `openrac_f2i`, which does
the same.

**a** for a float with exponent 255. The host reads it as a NaN, and both conversions gave
`0x80000000` (or, for `vftoi`, whatever the host's conversion of a NaN is), where the EE and the
VU saturate by the sign: `0x7FFFFFFF` for a positive one. That `0x80000000` is the value the
engine's texture cache takes for "very close". Fixed in `79868e57`. Test:
`test_conversions_saturate_by_sign`. `openrac_f2i`, the matched C's conversion, is left as it
was: a NaN there is as likely to be one the host made out of an overflow (whose sign differs
between x86 and ARM) as a value read from data, and it is not the case the texture cache hit.

## 6. VCALLMSR uses CMSAR0

**c.** No VU0 microprogram runs in the port: a function with `vcallms` or `vcallmsr` stays a
stub and is reported (rac1/pal has five, rac4 132), and where one matters the port has C for
what the microprogram computes. A `ctc2` to CMSAR0 (register 27) does nothing.

## 7. INTC_STAT and SetVSyncFlag

**b.** The register pages are plain memory ([guest.h](../../port/runtime/include/openrac/guest.h)),
so a VBlank wait that clears INTC_STAT's bit and polls it would not wait. No such wait runs:
rac1/pal's `func_00118ED0` and `func_00118F60` (libgraph's waits) are reached only from
`sceGsSyncV`, which the port replaces with its own end of frame (present, pace, run the VBlank
handlers: `libraries.tsv`), and no other function calls them (the generated report's call
graph); the same holds for rac1/ntsc's `VSync`. `SetVSyncFlag` stores the flag and the status
word at once, while the caller and its stack variables are still there
(`openrac_lib_SetVSyncFlag`, `03dff78c`), so nothing is written into a later stack frame.

## 8. VU0 macro-mode memory operations

**a.** `vlqi`, `vsqi`, `vlqd`, `vsqd`, `vilwr` and `viswr` made the function a stub (rac4 has a few).
Since `b9046220` they reach VU0's data memory, which the port's game memory has at the
EE's address for it, `0x11004000`, at `(vi & 0xFF) * 16`: never main RAM. `vsqi`/`vsqd` store `fs`
at the address in `it`; the increment comes after, the decrement before, both 16-bit, and
`vi0` stays 0. What the console's VIF0 would have put there by DMA is not there (no VIF0 runs),
so this serves code that writes and reads VU0 memory itself. Test:
`test_vu0_macro_memory_and_vf0`.

## 9. VU0 macro-mode flags

**a.** `vclipw` and a `cfc2` of `$vi16` (STATUS), `$vi17` (MAC) or `$vi18` (the clip flag) made the
function a stub (three in rac1/pal, sixteen in rac4). Since `b9046220`:

- `vclipw.xyz` shifts the clip flag up six bits and sets bit 0 for x > +|w|, bit 1 for
  x < −|w|, then y and z; `cfc2`/`ctc2` of `$vi18` read and write it.
- In a function that reads MAC or STATUS, every FMAC operation (the adds, subtracts,
  multiplies and their accumulator forms, `vopmula`, `vopmsub`; not `vmax`, `vmini`, `vabs`,
  `vmove`, the conversions) sets MAC from the fields it writes (zero, sign and overflow per
  field, x the top bit of each group) and STATUS's four summary bits and their sticky copies;
  `vdiv`, `vsqrt` and `vrsqrt` set STATUS's invalid (`0x10`) and divide-by-zero (`0x20`) bits and
  theirs. `ctc2` to STATUS writes the sticky bits. Elsewhere the flags are not kept, which
  saves every vector operation of the other functions the work: the game reads them right
  after its own operations.
- Underflow is not reported: with denormals flushed (item 1) such a result is already zero.

Test: `test_vu0_flags_and_clipping`. The console form of a translation (`--asm-console`, for
rac1-decomp's `tools/fcheck.py`) moves the three flag registers in and out with the others.

## 10. MMI semantics

**b** for what was there: the lane operations of `openrac_ee_mmi` were checked against the
list (`pexeh`, `pexch`, `prevh`, `pexew`, `pexcw`, `prot3w`, `pinth`, `pinteh`; `padduh` and `psubuh`
saturate; `pabsh` and `pabsw` of the minimum saturate; `pmfhi`, `pmflo`, `pmthi`, `pmtlo` move 128
bits; `pmulth` and `pmfhl.lw`, `.uw`, `.sh`).

**a** for what made a function a stub: `pmultw`, `pmultuw`, `pmaddw`, `pmadduw`, `pmsubw` (two
64-bit results in HI:LO and HI1:LO1 and rd, not a sum), `pmaddh`, `pmsubh`, `phmadh`, `phmsbh`
(the odd words of `phmadh` take the higher product, those of `phmsbh` its complement, as the
R5900 leaves them), `pdivw`, `pdivuw`, `pdivbw`, `psllvw`, `psrlvw`, `psravw` (shift amounts from
rs, results sign-extended to 64 bits), `padsbh`, `pmfhl.slw`, `pmfhl.lh` and `pmthl.lw`. Since
`5b7f0765`. Tests: `test_parallel_word_multiplies_shifts_and_divides`,
`test_parallel_halfword_multiply_add`.

## 11. VIF UNPACK V4-5

**b** for the size: the frontend's walk over the game's DMA lists steps over an `UNPACK` by
its size, and V4-5's is two bytes an element (`direct_packets`). **c** for the contents: the
port does not run the VU1 programs the unpacked data is for; the world, the mobys and the
effects are drawn by the port's own renderers from what the game holds in memory, and no
renderer reads V4-5 data. One that does must expand it as R = `(p & 0x1F) << 3`,
G = `(p & 0x3E0) >> 2`, B = `(p & 0x7C00) >> 7`, A = `(p & 0x8000) >> 8`.

## 12. The GIF is a stream

**b.** `GifInterpreter::gif` keeps a tag's unfinished data and joins it with the next transfer
(`910fef95`), so an `IMAGE` whose data runs on into the next DIRECT, or the next packet, is read
whole; image data continues only where a transfer was opened (TRXDIR), and only with the data of
the following DIRECT or DIRECTHL. The DMA walk reads the upper eight bytes of every tag as VIF
codes, as a list sent with TTE has them (`direct_packets`), so a `REF`'s `NOP` + `DIRECT` is not
taken for image data.

## 13. Progressive XGKICK

**c.** No VU1 program runs; nothing XGKICKs. The port's renderers draw from the game's own
data.

## 14. Kernel threads

**c.** The port runs the game on one thread. `CreateThread` and `StartThread` are only used by
the movie player, which the port replaces; `StartThread` logs that threads do not run
(kernel.c). Priorities and time slicing do not arise; interrupt handlers (the VBlank's, the
DMA's) run at set points of the frame on the game's own stack.

## 15. SetupHeap

**b.** The game's stack is outside its 32 MB in the port, so `EndOfHeap` gives the end of RAM
(`openrac_lib_EndOfHeap`), never less than the console would: an allocation like Deadlocked's
`malloc(0xc2500)` does not fail for want of heap. rac4 keeps its own explicit limit
(`rac4EndOfHeap`, port/game/rac4-ntsc).

## 16. PAL timing

**b.** The frame pacer holds the game to its standard's rate, 50 Hz for PAL
(`field_rate`, [clock.cpp](../../port/platform/clock.cpp)), whatever the display's. A late frame
counts the fields that went by, as the console's VBlank counter would, so the game's own
frame skip sees them; it does not run extra ticks to catch up, and one more than eight fields
late starts the schedule again from now (`kResyncFields`). The interpolated presentation of
[SETTINGS.md](SETTINGS.md) draws between ticks without changing their number.

## 17. IOP RPC handlers that block

**c.** There is no IOP. `sceSifCallRpc` answers the game's calls in host code
([kernel.c](../../port/game/common/lib/kernel.c)), and the sound engine is the port's own
([snd.c](../../port/game/common/lib/snd.c), [sound_engine.cpp](../../port/game/common/lib/sound_engine.cpp));
nothing waits on another IOP thread.

## What to check in play

The fixes of items 3, 8, 9 and 10 turn functions that were stubs (logged as missing when they
ran) into translations that run. That is what they are for, and it can also bring out the next
bug in the code they feed ([RAC1_PAL_BUGFIX_GUIDE.md](RAC1_PAL_BUGFIX_GUIDE.md), "making a dead
path live"). On 2026-10-20, with the owner's discs and decompilations, they were:

- rac1/pal: `func_L00_001FD428` and `func_L01_002A0D78` (`vclipw`); a third, `func_L00_00257EBC`,
  now stops at an input register no caller passes instead.
- rac4: `func_001482D0` (`vlqi`); `func_0046DB80`, `func_0046DBC8`, `func_0046DC28` (`viswr`);
  `func_0048C5C0` (`cfc2 $vi17`, MAC); `func_0044BFA8`, `func_00453EF0`, `func_004723E0`,
  `func_004DDD70`, `func_004DE210`, `func_004DE3B0`, `func_004DE628`, `func_004DE638`,
  `func_004DE698`, `func_004DF8F0`, `func_004E3030`, `func_004E3038` (`vclipw`); `func_004E4000`
  (`pmfhl.lh`); `func_004EE4A8` (`pmaddh`).
- rac1/ntsc, rac2 and rac3: none.

The denormal flush (item 1) applies to the whole game thread of every game.
