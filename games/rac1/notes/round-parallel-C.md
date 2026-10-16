# Range C (0x20CC00-0x21FFFF) — harvesting the newly-unblocked sq/lq pool

> **Historical round notes (September 2026), kept as a record.**
> `docs/DECOMP_PROGRESS.md` and `progress/report.json` hold the current
> state. Superseded since this was written:
> - The remaining size mismatch named here, `func_00115098`, has been
>   reverted.
> - The `movz`/`movn` correction was adopted: `tools/rank_candidates.py`
>   no longer blocks on `movn`.
> - On `.align 4`: `tools/sweep_matches.py` now reports only padding of
>   more than 4 bytes, the case where retail wanted 16-byte alignment (see
>   "Post-`endlabel` padding" in `docs/DECOMP_PROGRESS.md`).

## Range survey (205 remaining stubs, mechanically classified)

| Count | Category |
|---|---|
| 84 | **`sq` only — newly unblocked, the real pool** |
| 30 | fragments (no `jr $31`) |
| 18 | clean candidates (no blocker at all) |
| 15 | `gp`+`sq` |
| 13 | `movnz`+`sq` |
| 11 | `gp`+`movnz`+`sq` |
| 6 | `movnz` only |
| ~28 | handwritten / VU / quad / mixed |

So ~102 of 205 (50%) are viable now, against the old assumption that
`sq` meant blocked. Confirms the misclassification warning in
DECOMP_PROGRESS.md for this range too.

## Matched (byte-exact)

| Function | Size | C |
|---|---|---|
| `func_00216270` | 0x20 | `func_0012F068(func_002177F0);` — passes a function *address* as the arg (`lui`/`addiu` of `%hi`/`%lo`) |
| `func_00218908` | 0x20 | `func_00217F68(D_0013CA40);` |
| `func_0021DAE0` | 0x20 | `func_00226D50(1); return 0;` — the `daddu $2,$0,$0` after the `lq` is the `return 0` |

All three are `$ra`-only-saving wrappers that spill with `sq $31`/`lq $31` —
exactly the shape v1.36 could never emit and v1.14 does. They are the
first confirmation that the per-segment build actually opens this pool
in practice, not just in principle.

Note these are "last statement is a bare call" shapes, which the old
skip list warned about for tail-call risk. That risk did not materialise:
retail keeps a real frame here, and v1.14 does too, so they compile fine.
The tail-call skip category should be read as "retail tail-calls but we
don't", not "the last statement is a call".

## Finding: two known false-positives were causing text-wide address drift

`func_001F9B90`/`func_001F9B98` (the `max.s`/`min.s` inline-asm wrappers)
were already documented as size-mismatched non-matches (+4 bytes each,
caught by the new size check). They were also silently shifting every
later function in `text`: all three of my functions initially showed
1-2 byte diffs that were purely `jal`/`%lo` address fields pointing 16
bytes late.

Reverting both to `INCLUDE_ASM` cleared the drift completely and all
three of mine went to 0/32. Whole-project sweep after: **143 -> 147
exact**, size mismatches **3 -> 1**. So the revert fixed more than the
two functions themselves.

This is the `text`-segment analogue of the old `core_text` "systemic
artifact", and the lesson generalises: **a size-mismatched function is
not a local problem — it silently corrupts verification for everything
after it in the same object.** Worth treating any surviving size
mismatch as high priority rather than as a documented curiosity. The one
remaining (`func_00115098`, -4, in `core_text`) is outside my range.

## Second batch (7 more exact, 1 documented-close)

| Function | Size | Result | Note |
|---|---|---|---|
| `func_0020E040` | 0x28 | 0/40 | `func_001F99B0(0x70003A00, 0x40000000, 0x380)` — scratchpad address + length |
| `func_0020E068` | 0x2C | 0/44 | `func_001F9A98(D_001B3200, 0x70003A00, 0x380)` |
| `func_0020E098` | 0x2C | 0/44 | same callee, buffer/scratchpad args swapped — a to/from pair |
| `func_0021FAF8` | 0x30 | 0/48 | **first true s-register spill match** (`$16`) |
| `func_0021F200` | 0x38 | 0/56 | float field via `func_001FA748(field, 0.01f)`; `0x3C23D70A` is `0.01f`, and it lands in `$f13` = 2nd float arg |
| `func_0021EF60` | 0x3C | 0/60 | two-field sibling of `func_0021FAF8` |
| `func_002140B0` | 0x3C | 0/60 | `((func_001160D8() >> 16) & 0x7FFF) % arg0` — real `div` from a plain `%`, per the no-strength-reduction finding |
| `func_00217588` | 0x40 | 2/64 | allocator register-choice; kept documented-close |

## Third batch (3 more exact) + a skip-category correction

| Function | Size | Result | Note |
|---|---|---|---|
| `func_00217A60` | 0x6C | 0/108 | see correction below |
| `func_00217860` | 0x5C | 0/92 | needed branch polarity flipped |
| `func_002178C0` | 0x5C | 0/92 | sibling of the above |

### Correction: `func_00217A60`'s `movn` was never a real blocker

It had been filed under the `movz`/`movn` recognise-and-move-on skip
category. It is not a codegen-heuristic case at all — the
`slt`/`addiu +3`/`movn`/`sra 2` sequence is just the **standard signed
divide-by-4 idiom**, which a plain `x / 4` in C reproduces exactly.
Byte-exact 0/108 first attempt once written that way.

**So the `movz`/`movn` skip category is over-broad and is hiding viable
functions.** Before skipping on `movn`, check whether it is one of the
signed-division idioms (`/ 2`, `/ 4`, `/ 8`...: `slt` against -1, add
`divisor-1`, conditional-move, arithmetic shift). Range C alone still
lists 13 `movnz,sq` + 11 `gp,movnz,sq` + 6 `movnz` stubs, some of which
are probably this same idiom.

### Branch polarity, restated usefully

`func_00217860`/`func_002178C0` were size-mismatched (54/92) with the
`arg0 == 0` case written as the `then` arm, and byte-exact once flipped
so the `then` arm is the path retail **falls through to** (here
`arg0 != 0`), with the branch-taken case as the `else`. This is the
existing branch-polarity technique, but the reliable way to read it off
the disassembly is: whichever path directly follows the conditional
branch is the `then` arm.

## Cumulative for this round: 13 exact, 1 documented-close (2/64)

Whole-project sweep at end of round: **172 decompiled, 158 exact**, 1
size mismatch (`func_00115098`, `core_text`, pre-existing), 13 byte
mismatches (all pre-existing documented near-misses; nothing I added
regressed).

## Attempted and reverted: `func_0021D9C8` (best 18/152)

Logic is fully worked out, recorded here for whoever picks it up:

```c
int func_0021D9C8(char *arg0) {
    int *dst = (int *)(arg0 + 0x30);
    int *src = D_00141FA0;
    int i = 7;
    int n;
    do { *dst++ = *src++; } while (--i >= 0);   /* 8 words */
    *(int *)(arg0 + 0x50) = 0;
    if (*(int *)(arg0 + 0x30) != 0) {
        int *base = (int *)(arg0 + 0x30);
        do {
            n = *(int *)(arg0 + 0x50) + 1;
            *(int *)(arg0 + 0x50) = n;
            if (base[n] == 0) break;
        } while (n < 8);
    }
    *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) % 8;   /* the div idiom */
    return 0;
}
```

The tail `% 8` confirms the div-idiom reading again (`slt` vs -1, `+7`,
`movn`, `sra 3`, `sll 3`, `subu`). The residual 18/152 is in the two loop
bodies, not the tail. Reverted rather than kept, since 12% is not a
small isolated diff. Note retail re-loads `arg0+0x50` every iteration
rather than caching it, which is the redundant-reload signature — worth
trying a `volatile` field here.

## ⚠ Finding: the `.align 4` padding fix is NOT universally correct

The hardened `sweep_matches.py` (from the coordinator) reports functions
whose `.s` carried post-`endlabel` padding and advises adding
`__asm__(".align 4");` after the C, on the grounds that the padding
vanishes when the function stops being an `INCLUDE_ASM` stub and
everything after it shifts.

7 of my 13 functions were flagged that way. **Adding the directive broke
them**: they went from 0/N to 1-2/N, with the diffs being `jal`/`%lo`
address fields — i.e. the directive *introduced* drift rather than
removing it. Removing it again restored all 13 to byte-exact, with the
whole-project sweep showing 158 exact and no regressions either way.

So on this evidence the padding is already accounted for (presumably by
the compiler's own function alignment, given these sizes), and the
advice should be applied **only when verification actually shows drift**,
not pre-emptively on the basis of the `.s` having had padding. Treating
the sweep's padding list as a to-do list would silently damage working
matches.

Recommend the sweep's wording be softened from "confirm each has one" to
"check whether one is needed", since the flag is a heuristic, not a
defect.
