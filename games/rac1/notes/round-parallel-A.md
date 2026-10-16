# Range A (`src/text.c`, vram 0x1E9080-0x1F9B00)

> **Historical round notes (September 2026), kept as a record.**
> `docs/DECOMP_PROGRESS.md` and `progress/report.json` hold the current
> state. Superseded since this was written:
> - "`$gp`-relative addressing ... at `-G0` neither is reachable": the
>   build uses `-G2` now, and `$gp` functions match.
> - The recommended revert of `func_00115098` was done. The "-8 drift from
>   about `func_00234380`" was padding lost after `func_00234350`, since
>   restored with `__asm__(".align 4")`.
> - `func_001EC270` and `func_001EC780`, kept here at 1/68, are now exact.
> - `src/text.c` and `src/core_text.c` no longer exist: the source is
>   split into `src/core/` (one file per retail object) and `src/game/`
>   (one file per original source file).

## This file's previous contents were WRONG - corrected here

An earlier survey in this file concluded that **95 of 165** remaining
stubs in this range were "`sq`/`lq`-blocked", that only **6 candidates**
existed, and that the range was **93% exhausted and should not be
re-scanned**. Anyone acting on that would have written off most of the
range.

**Why it was wrong.** It compared our compiler's output against the
project's then-current claim that "retail always spills callee-saved
registers as `sd`/`ld`" - itself a bad generalisation from a single
`core_text` function. Retail's two segments were built by two different
SN sub-builds: `core_text` by v1.36 (`sd`/`ld`), and **`text` - this
range - by v1.14, which uses `sq`/`lq`**, exactly what we now emit for
`text.c`. So a `sq`/`lq` spill here is *correct output*, never a
blocker. The survey was measuring against a sentence in the docs
instead of against retail's actual bytes.

## Corrected survey

Produced mechanically by the new `tools/survey_range.py`, which reports
spill style as information and never as a blocking category:

| Count | Category | Old count |
|---|---|---|
| **65** | **candidate** | 6 |
| 46 | `$gp`-relative addressing | 16 |
| 22 | fallthrough fragment (no `jr $31`) | 22 |
| 21 | handwritten asm (spimdisasm-marked) | 21 |
| 7 | `movz`/`movn` heuristic | 3 |
| 2 | SIMD/COP2 | 2 |
| 1 | varargs prologue | - |
| 1 | bare quadword | - |

**6 candidates -> 65.** The old 95-strong `sq`/`lq` bucket splits
roughly: about 30 are genuinely `$gp`-relative (that check now runs
*before* the spill check, whereas the old survey tested spills first and
stopped there), and the remainder are real candidates.

`$gp`-relative is now the largest genuine blocker here at 46, and it
covers **loads as well as stores** - at `-G0` neither is reachable from
plain C.

## Matched this round

| Function | Result | Notes |
|---|---|---|
| `func_001F6600` | **0/32** | Wrapper: `func_001F65B0(arg0, arg1, D_001DF3D0)`. First attempt. |
| `func_001F6620` | **0/32** | Same, `D_001DF770`. |
| `func_001F6640` | **0/32** | Same, `D_001DFB10`. |
| `func_001F7B40` | **0/44** | Three sequential void calls. Needed the padding fix below. |
| `func_001F2FB8` | **0/80** | Dispatch on `D_0018C434`: 0 -> `func_001F99B0(D_001940C0,-1,0x80)`, 2 -> `func_001F2BC8()`. |
| `func_001EC270` | 1/68 | Vtable dispatch, kept as documented-close. |
| `func_001EC780` | 1/68 | Identical twin on the `+0x10` slot; residual predicted before compiling, then confirmed. |

All three `func_001F66xx` wrappers spill `$ra` via `sq`/`lq` and sat in
the old "blocked" pile - direct proof the category was misclassified
rather than merely doubtful.

Sweep went **143 -> 148 exact**, size mismatches **3 -> 1**, no
regressions.

## Two layout traps (both corrupt *other* functions, not their own)

**1. Content after `endlabel` is inter-function padding the stub was
supplying.** `func_001F6640.s` carries two trailing `nop`s after its
`endlabel`. While it was an `INCLUDE_ASM` stub those 8 bytes were part
of the object; converting it to C dropped them and shifted every later
`text` function by -8, so `func_001F7B40` reported 1/44 despite being
instruction-for-instruction identical to retail. Fixed by emitting the
padding explicitly next to the function.

Two refinements worth knowing:

- The *following* stub's `.align 3` absorbs such padding automatically
  when the gap ends at the next 8-byte boundary - `func_001EC270.s`'s
  single trailing `nop` at a non-aligned address needed no fix at all.
  Explicit padding is only required when the gap *starts* already
  8-byte aligned, as `func_001F6640`'s did.
- `tools/sweep_matches.py` in the main checkout now detects this class
  and suggests `__asm__(".align 4")` as the general fix. That is right
  for the common case (retail aligning the next function to 16 bytes)
  but does **not** cover this instance: the gap here runs 0x1F6660 ->
  0x1F6668, and 0x1F6660 is already 16-byte aligned while 0x1F6668 is
  not, so no alignment directive can produce it. Explicit `nop`s are
  needed when the gap is not an alignment artifact.

**2. A non-matching function that is LONGER than retail poisons every
downstream address.** `func_001F9B90`/`func_001F9B98` (max.s/min.s
inline-asm wrappers) were kept as documented-close C while each ran 4
bytes long, quietly adding 8 bytes of drift to the whole segment.
Reverted to `INCLUDE_ASM`, restoring the stubs' exact retail bytes.

**Rule this establishes:** same-size byte-diff near-misses are harmless
to keep; **size-mismatched ones must be reverted**, because they break
verification for every function after them. Same failure mode as the old
`core_text` `func_00112380` drift.

## Reverted, logic confirmed

`func_001E9E70` at 20/88. Body is
`func_0022C7E0(); func_0022C188(); func_0022C870();
func_00234C98(0x47, 0x5360B);
func_00234C98(0x4E, 0x1000000 | (D_0015EF88 >> 13));`.
The instruction multiset is correct but the order is not: for both calls
retail schedules the *first* argument's `addiu $4` into the `jal` delay
slot and materializes `$5` beforehand, while this compiler does the
reverse. Argument-materialization order driving delay-slot choice - an
instance of the delay-slot question, and not reachable by reordering the
C, since the arguments are constants inside a single call expression.

## Reusable technique notes

- **Don't fold a field offset into a computed address.** Writing
  `base + idx * stride + 8` folds the `+8` into the `%lo` constant;
  retail keeps it as a `lw` offset. Compute the record pointer, then
  read `rec + 8` separately (`func_001EC270`: 7/68 -> 3/68).
- **`rec += idx` puts the sum in the base's register**, where an
  initialiser puts it in the index's (3/68 -> 1/68). Confirms the
  documented in-place-accumulate lever.
- **Twin functions let you predict the residual.** `func_001EC780` is
  `func_001EC270` with a different slot; predicting 1/68 before
  compiling and having it confirmed is a cheap check that the residual
  really is the allocator question and not a misread.

## Recommendations beyond this range

- **`func_00115098` (core_text) should be reverted.** Still
  size-mismatched (retail 60, ours 56), causing -4 drift through
  `core_text` by trap (2). Untouched only because `core_text.c` is
  outside this range's remit.
- **A -8 drift exists in `text` from about `func_00234380` onward**,
  predating this round and originating outside this range. It currently
  costs `func_00234350` and `func_0023DFC0` a byte each, and about 2 of
  `func_001E9E70`'s 20. Likely a converted stub that dropped
  post-`endlabel` padding.
- **`tools/survey_range.py` is reusable** for any range/segment. Other
  ranges whose "blocked" counts leaned on the `sq`/`lq` generalisation
  should be re-run through it - range B reported 69 `sq`/`lq` and range
  D reported 209, and both are `text`.
