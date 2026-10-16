# Parallel round, agent B — vram 0x1FB600–0x20CB00 (src/text.c)

> **Historical round notes (September 2026), kept as a record.**
> `docs/DECOMP_PROGRESS.md` and `progress/report.json` hold the current
> state. Superseded since this was written:
> - "`$gp`-relative ... unreachable at `-G0`": solved by `-G2`.
> - The two "unsteerable" register sub-cases (`%hi` reuse on a global
>   load, `$at` for a global store) were the assembler macro form, solved
>   by `MACRO_ADDR`. `func_00209808`, `func_00209858`, `func_002098C8`,
>   `func_00209918`, `func_00209698` and `func_002094E0` are now exact.
> - The "remaining lead" (8 bytes short between 0x20BAA8 and 0x23DFC0) was
>   the padding lost after `func_00234350`, since restored.
> - `src/text.c` and `src/core_text.c` no longer exist: the source is
>   split into `src/core/` (one file per retail object) and `src/game/`
>   (one file per original source file).

Findings for the coordinator to merge into `docs/DECOMP_PROGRESS.md`.

## ⚠ The survey immediately below is WRONG — see "Corrected survey" at the end

Its headline claim, "~79% of this range is blocked", rests on counting
69 functions as blocked by `sq`/`lq`. They were never blocked. Retail's
`text` segment was built with SN v1.14, which spills callee-saved
registers as `sq`/`lq` — exactly what we now emit. They were being
compared against this project's since-corrected "retail always uses
`sd`" generalisation rather than against retail's actual bytes. Kept
below only to show what the mistake looked like.

## Range survey (SUPERSEDED — see the correction above and below)

189 `INCLUDE_ASM` stubs remain in this range. Classified them all
mechanically (by grepping each `.s` for the relevant signature) rather
than one at a time:

| Category | Count |
|---|---|
| `sq`/`lq` callee-saved spills (unsolved gap) | 69 |
| `$gp`-relative (`($28)`) addressing | 53 |
| **real candidates** | 40 |
| fallthrough fragments (no `jr $31`) | 17 |
| `movz`/`movn` heuristics | 6 |
| handwritten (spimdisasm-marked) | 3 |
| SIMD/COP2 | 1 |

So **~79% of this range is blocked by known categories**, and the two
dominant blockers are the `sq`/`lq` question (69) and `$gp`-relative
addressing (53). Worth noting for prioritisation: solving either would
unblock far more of this range than any amount of per-function work.

One classification refinement: the existing skip category is written as
"`$gp`-relative **stores**", but `$gp`-relative **loads** are equally
unreachable — we compile `-G0`, so no gp-relative addressing is
generated at all. Several functions here are load-only (e.g.
`func_002073A8`, `func_001FFFA0`). Suggest widening that category's
wording to "`$gp`-relative addressing" generally.

Also: the 4-byte functions in this range are **not** the
`0xCDCDCDCD` padding pattern — all 13 are single-instruction
fallthrough fragments with no `jr $31` (e.g. `func_00208D30` is a lone
`sw $4, 0x1C($2)`). Same "not a standalone function" category as
`func_00113AD8`, different signature from the padding one.

## Per-function results

| Function | Status | Notes |
|---|---|---|
| `func_0020BAD8` | **matches** (0/56) | Byte-exact, first attempt. Pointer-walk loop over a record list: running total starts at 8, each iteration adds 8 plus the record's length field at `+4`, advances the pointer `0x10`, and 4-byte-aligns the total (`(n + 3) & ~3`); loop continues while the record's first word is nonzero; returns total + 8. Plain `int`/`& ~3` gave retail's `addiu $6,$0,-4` mask directly (no unsigned form needed here). |
| `func_00203B18` | **matches** (0/88) | Byte-exact after two documented-technique fixes. Relocation/fixup routine: takes a table base and index, loads the object pointer at `+0x48`, rebases the `+0x14` field if nonzero, then rebases each of the `[0x10]`-count pointers starting at `+0x1C`. The count is legitimately re-loaded every iteration (the `int *` stores can alias the byte count — natural aliasing, **not** a `volatile` case, worth distinguishing from the documented volatile signature). First attempt was 11/88 with only 3 words differing; both were textbook: (1) retail accumulates the index into `arg0`'s own register (`addu $4,$4,$5`), fixed by writing `arg0 += idx * 4;` as an in-place accumulate rather than indexing a cast array — the `func_0011AA68` technique; (2) retail puts `i = 0` in the `beqz` delay slot rather than the `p = obj + 0x1C` setup, fixed by ordering `i = 0;` before `p = ...` in the source. |
| `func_00209808`, `func_002098C8`, `func_00209918` | **close, not exact (9/80, 11.3% each)** — kept as documented-close | All three are **byte-for-byte identical to `func_00209858`** in retail — four copies of the same function. Same C body, same residual, same cause. Includes the `func_00209808` retry the directive requested: the prior round's 24/80 was **not** a register-steering problem, it was the folded address constant (see `func_00209858` below). |
| `func_00209858` | **close, not exact (9/80, 11.3%)** — kept as documented-close | Zeroes a record's status field if it equals 2, then sets an error code unless a flag bit is set. **The fix worth propagating:** retail reads the index off the struct base and *then* advances that base by `0xB0` as a separate `addiu`; writing the advance into the declaration (`char *s = D_0013D390 + 0xB0;`) makes this compiler fold both into a single combined address constant, a visibly different instruction — that alone was the difference between 51/80 and 9/80. All 9 remaining bytes are the two established unsteerable register sub-cases (`%hi`-reuse on the `D_0015EFB4` load; `$at` for `D_0015EFB0`'s `%hi` with the literal ordered before it), same as the `func_002094E0`/`func_00209698` family. |
| `func_00209698` | **close, not exact (13/64, 20.3%)** — kept as documented-close C | Direct sibling of `func_002094E0`: same guard (field `0xDC == 2` && status field `0xE4 < 0`), same three writes, only the constants differ (9 / `0xF` here vs 7 / `0xB` there). Landed on *exactly* the same residual and cause as that function — retail stores the two struct fields first, then computes `D_0015EFB0`'s address using `$1`/`$at` for its `%hi`; this compiler materializes that address earlier and stores it before the second struct field. Established two-base store-order/`%hi` question. Not re-tried against source reordering because `func_002094E0`'s entry already records that as ineffective. Kept on that sibling's precedent (identical ratio). |
| `func_00208208` | **reverted** (best 16/48, 33%) | Logic confirmed correct: `return arg1 >= 0x141 \|\| 63.5f <= f2;` — an int bound OR a float threshold, where the float is the **third** float parameter (lands in `$f14`; the 2nd float lands in `$f13`, so the signature is `(int, int, float, float, float)`). Two source-shape fixes did land real progress: writing the comparison constant-first (`63.5f <= f2`, not `f2 >= 63.5f`) reproduced retail's `c.le.s $f0,$f14` operand order exactly, and using `if (...) return 0; return 1;` (rather than a direct boolean return) fixed the branch polarity so `1` is the fall-through default. Reverted at 33% per the threshold rule — see the new open-question note below for what's left. |
| `func_0020C210` | **skipped, likely handwritten** | Writes DMAC MMIO at `0x1000D400` using `$1`/`$at` as the base register (`lui $1` / `ori $1` / `sw` with offsets). A compiler never allocates `$at`; this is the same signal that got `func_0023C9B0` classified as hand-written, and it sits in the same DMAC/hardware cluster as the spimdisasm-marked handwritten functions. Not attempted. |
| `func_00201AE0`, `func_00201948`, `func_001FBC78`, `func_001FE4C0`, `func_001FE580`, `func_001FF660`, `func_001FF950`, `func_001FFD98`, `func_00200190`, `func_002008B0`, `func_00202258`, `func_00202EF8`, `func_00205218`, `func_00208858`, `func_00208D30` | **not standalone functions** | No `jr $31`; single instructions or short runs of `addiu $sp,$sp,N` stack adjustments. Fallthrough fragments. |
| `func_002073A8`, `func_001FFFA0` | **skipped, `$gp`-relative** | Both load/store through `$28`. Unreachable at `-G0`. |

## Proposed new open-question entry

**FP branch-likely: retail uses `bc1fl` where this compiler emits
`bc1f` + a fix-up.** Found on `func_00208208`. For
`if (intcond && floatcond) return 0; return 1;`, retail emits:

```
beqz   $5, END          # int test
 addiu $2, $0, 1        # delay (always) -> default 1
lui/mtc1 the constant
nop                     # mtc1 -> c.le.s hazard nop
c.le.s $f0, $f14
nop
bc1fl  END              # LIKELY: delay only runs when taken
 daddu $2, $0, $0       # -> 0
END: jr $31
```

this compiler emits a plain `bc1f`, whose delay slot always executes, so
it must add a second `li $v0,1` on the fall-through path to undo it. Same
total size (48 bytes), functionally identical, but 3 instructions differ.
Additionally retail has an `mtc1`→`c.le.s` hazard `nop` that this
assembler does not insert (ours does insert the `c.le.s`→`bc1f` one, so
hazard handling is on, just not for that pair).

This is distinct from the documented `movz`/`movn` conditional-move
question (that's branch-vs-conditional-move; this is
branch-vs-branch-likely) and from branch *polarity* (which is
source-steerable — the polarity here was fixed successfully). No source
shape tried produced a likely branch; a direct boolean return was worse
(19/48). Plausibly a `-mbranch-likely`-style codegen setting rather than
anything reachable from source, but that was not tested since the build
flags are fixed.

**Important narrowing of that finding:** branch-likely *is* reachable
from plain C in general — `func_00209858`'s `beql` implementing
`if (*rec == 2) *rec = 0;` matched retail exactly, no coaxing needed.
So the rule is not "this compiler won't emit likely branches". It emits
them for an integer `if` whose entire body fits the delay slot; it did
not emit one for the FP-condition case where the delay slot holds the
*default* value for a path that isn't the `if` body. Whoever picks this
up should treat it as an FP-branch / value-materialization question
rather than a general branch-likely gap.

## Suggested additions to the techniques library

Both of these are existing techniques whose *scope* this round extended,
rather than new ones — worth folding into the existing entries:

1. **Pointer-advance must be its own statement, not part of the
   declaration.** The existing "Splitting a large constant offset into
   pointer-advance + field offset" entry describes the retail shape; add
   that the C has to *separate* the advance (`s = base; ...; s += 0xB0;`)
   because putting it in the initializer (`char *s = base + 0xB0;`) lets
   the compiler fold it into one address constant. This was a 51/80 →
   9/80 swing on `func_00209858` and, retroactively, the real reason the
   earlier `func_00209808` attempt sat at 24/80.
2. **Re-loads forced by genuine aliasing are not the `volatile`
   signature.** The "Redundant reload + unfilled delay slot = volatile"
   entry is worth qualifying: in `func_00203B18` retail re-loads a byte
   count on every loop iteration purely because the loop's `int *`
   stores may alias it. Plain C reproduces that reload exactly with no
   `volatile`. So check for a legitimate aliasing explanation before
   reaching for `volatile`, or you will add a qualifier that changes
   other codegen.

---

# Second round (post-`sq`/`lq` fix, text.c now built with SN v1.14)

## Corrected survey

The `sq`/`lq` category was never a blocker for `text`. Honest breakdown
of the 177 stubs remaining in range now:

| Count | Category |
|---|---|
| 52 | `$gp`-relative addressing (loads *and* stores; unreachable at `-G0`) |
| **50** | **has `sq`/`lq` spill — NOW VIABLE, not blocked** |
| 33 | plain candidate (no known blocker) |
| 20 | `movz`/`movn` conditional-move heuristic |
| 17 | fragment (no `jr $31` of its own) |
| 5 | spimdisasm-marked handwritten asm |

So this range holds **~83 workable functions** (50 newly-unblocked + 33
plain), not the ~40 the old survey implied. **`$gp`-relative addressing
is now the single largest genuine blocker here** — 52 functions in this
range alone — and is plausibly worth the same focused attack that
cracked `sq`/`lq`.

## Matched

| Function | Result | Notes |
|---|---|---|
| `func_001FFD30` | **0/104 exact** | `volatile` on the reloaded pointer field **plus** materializing the global base *after* the call (see new technique). |
| `func_0020BAA8` | **0/48 exact** | `if (func_001236F0()) func_001E9730(D_001E8690);` |
| `func_00202790` | **0/44 exact** | `s0`-preserves-arg-across-two-calls. Needed a trailing `.align 4` — see padding trap. |
| `func_002071A8` | **0/40 exact** | 6-arg call (EABI passes args 5/6 in `$8`/`$9`) into the `func_00209048` orientation predicate with fixed screen coords. |

## New technique: where a global base is materialized, relative to a call

For a global used only *after* a call, the placement of its
materialization is itself a register-allocation lever:

- Initialized **before** the call → live range crosses the call → gets a
  **callee-saved** register and the function grows an extra spill pair.
  On `func_001FFD30` that was 85% mismatch and the wrong frame size.
- Assigned **after** the call → lands in a **temp** register, matching
  retail.

Combined with the `volatile` and direct-vs-base-pointer levers, this
took `func_001FFD30` from 85% to exact.

## Confirmed negative: v1.14 does NOT rescue the leaf near-misses

Rechecked as directed. `func_00209808`/`func_00209858`/`func_002098C8`/
`func_00209918` unchanged at 9/80; `func_002094E0`/`func_00209698`
unchanged at 13/64. Reason: all six contain **zero** spill instructions
(grep for `sq|lq|sd|ld` returns 0 — they are leaves), so which sub-build
compiles them cannot matter. The earlier `%hi`-reuse/`$at` diagnosis
stands; "they were compiled with the wrong compiler" does not apply to
leaves.

## Attempted and reverted

- `func_0020BB88` — 15/60 (25%), over threshold. Logic confirmed:
  `n = arg0[1]; result = 0; if (n) result = (func_0020BB10(arg0+2, arg0[0]) == n); return result;`
  Right size and instructions, but retail emits the `result = 0`
  (`daddu $2,$0,$0`) **inside the prologue**, between the stack adjust
  and the spills; this compiler always places it after the spills,
  shifting the rest. Tried single-result, early-return and if/else forms
  and both declaration orders — the last-statement-emits-first rotation
  rule does not reach prologue scheduling.

## Kept as documented-close

- `func_00205220` — 9/80 (11.25%), the same ratio as the already-kept
  `func_00209858` family. Logic confirmed; swapping the two globals'
  addition order took it 11/80 → 9/80. Residual is which registers the
  two global-address materializations land in (retail `$v1`/`$a0`, ours
  `$a0`/`$a1`).

## Traps

**Trailing padding vanishes when you decompile a function.**
`func_00202790`'s `.s` carried one padding word after `endlabel` (retail
aligns the next function to 16 bytes; splat emits only `.align 3`).
Decompiling drops it, shifting every later function by -4 and producing
spurious relocated-`jal` diffs *far from the cause*. Fixed with
`__asm__(".align 4");` after the function.

**Drift makes exact functions look wrong.** With +16 bytes of upstream
drift, `func_002071A8` read 1/40 and `func_00202790` 2/44 while being
instruction-for-instruction identical to retail; both went exact with no
source change once the drift source was removed. When the only differing
bytes lie inside `jal` targets, suspect drift before suspecting your C.

## Remaining lead

Something between 0x20BAA8 and 0x23DFC0 is still **8 bytes short**
(`func_0023DFC0` shows -8 drift) and it is not one of the sweep's
reported size mismatches — most likely another lost-padding case.
