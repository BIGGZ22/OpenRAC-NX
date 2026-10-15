# Round notes: cashing in the sign-extension solution

> **Historical round notes (September 2026), kept as a record.**
> `docs/DECOMP_PROGRESS.md` and `progress/report.json` hold the current
> state. Superseded since this was written:
> - `func_00217860` and `func_002178C0` ("blocked on the unsolved sq/lq
>   question") and `func_00217A60` ("hinges on movn") are now exact;
>   `func_00217A08` was attempted and reverted (size mismatch).
> - The recommended size check was added: `tools/check_match.py` fails
>   loudly on any size disagreement. `func_001F9B90`/`func_001F9B98` were
>   reverted.
> - `func_0011EEC8` and `func_0011DFE8` turned out to be libgcc
>   (`__muldi3`, `__divdi3`) and are exact from GCC's own source (see
>   `src/libgcc/README.md`).
> - `src/text.c` and `src/core_text.c` no longer exist: the source is
>   split into `src/core/` (one file per retail object) and `src/game/`
>   (one file per original source file).

Branch `parallel-C`. All results below verified with the compiler's own
exit code checked as 0 first, then against a freshly linked
`build-sn/rac1.elf`.

## Headline: the systemic drift is GONE

`func_00112380` is byte-exact (0/40). The `dsll32`/`dsra32` pair is the
**callee returning 64-bit `long`, narrowed to `int` at the call site**:

```c
extern long func_00116F68(int, int, int);
int func_00112380(int arg0) { return (int)func_00116F68(arg0, 0, 10); }
```

**The "Known systemic artifact" warning at the top of the status table
can be deleted.** Verified: drift across `core_text` is now zero (walked
every function symbol's offset against its retail offset — no divergence).

### 5 entries auto-upgraded from "matches (mod. known drift)" to exact

Re-verified individually, all now 0/N: `func_001144F0` (0/36),
`func_00116408` (0/32), `func_00119798` (0/36), `func_00119840` (0/36),
`func_0011D078` (0/28). Their "(mod. known drift)" qualifiers should be
dropped.

### The 6th was misdiagnosed

`func_00113A70` — its entry blamed "4 relocated addresses / known drift".
Wrong: with drift at zero it still showed 8/88, and the real cause was
**store order**. The documented rotation rule (last source statement
emits first) gives it directly — moving the `+0x0` store to last in
source makes it **0/88**. Entry needs correcting.

## NEW FALSE-POSITIVE CLASS: oversized functions read as matches

`func_0011AE1C` was recorded as a **match** but is not one. Retail is a
bare 4-byte `jr $31` with nothing in its delay slot (the next function's
first instruction occupies it). `void f(void) {}` emits `jr $ra; nop` —
8 bytes — and GCC additionally force-aligns a compiled function to 8
bytes where retail sits at a 4-aligned address. 8 bytes of drift total.
Reverted to `INCLUDE_ASM`.

**Why it passed:** `tools/check_match.py` compares exactly `retail_size`
bytes. The first 4 bytes (`jr`) matched and the extra `nop` was simply
outside the comparison window. **Any function that is correct in its
first N bytes but LONGER than retail reports a false match this way.**
This is distinct from the known stale-object false positive.

This one was also the *actual* blocker on landing `func_00112380`:
adding the correct 8 bytes overflowed `.core_text` into `.core_data`,
because this function's excess had been silently absorbed by
`func_00112380` being 8 bytes short. Two bugs cancelling out.

### Two more instances found, still outstanding

A full sweep of every compiled function in both files (size **and**
bytes) found two more oversized-by-4 functions in `text.c`:
`func_001F9B90` and `func_001F9B98` (retail `0x8`, ours `0xc`). They
don't currently break the link (text.c has room) and position-independent
`check_match.py symbol` still verifies later functions correctly, so
this is latent rather than breaking — but their recorded status should be
re-examined, and they contribute +8 of layout drift within `text.c`.

**Recommendation: teach `check_match.py` to compare sizes too**, and fail
loudly when our function is longer than retail. That closes this class.

## Newly matched (8 total this round)

Priority targets:
- `func_00112380` — 0/40 (see above)
- `func_00113A70` — 0/88 (rotation rule)
- `func_0022F090` — 0/24. Pair on `$5`, so `(int arg0, long arg1)` with
  `int *p = (int *)(int)arg1;`
- `func_0022F0F0` — 0/44. Same signature fix **plus** the rotation rule:
  its three guarded stores emitted as `(0x1C, 0x4, 0x18)` from natural
  order, so source is written `(0x18, 0x1C, 0x4)`. Its old entry
  predicted the unfilled `beqz` delay slot would need explaining — it
  didn't; natural C reproduces it once the parameter is typed right.

Then a sweep for the prologue idiom (`dsll32`+`dsra32` on `$4`-`$7` in
the first few instructions) found 13 such functions, 10 still
`INCLUDE_ASM`, mostly one cluster at `0x2178xx`-`0x217Axx` — the family
of the already-matched `func_00217830`. Landed 4:

- `func_002179C8` — 0/60. Needed branch-polarity: natural
  `if (arg0 == 0) ... else if` inverted the second branch; written with
  `arg0 != 0` as fall-through and the zero case in a trailing `else`.
- `func_00217920` — 0/76. Same shape a level deeper. **Note:** it stores
  the *loaded* `p[5]` value (still live in `$4`) to `D_001517F0`, not the
  literal `1` it was just compared against — keeping it in a local
  reproduces the register reuse; writing `= 1` emits an extra `addiu`.
- `func_00217970` — 0/84. The `bltzl` branch-likely idiom (delay slot
  annulled when not taken) reproduces naturally from a plain
  `if (arg0 < 0) *(int *)p = arg0;` — no special handling needed.
- `func_0022F0A8` — 0/68. Same shape as `func_0022F0F0`, with `lbu` so
  `unsigned char *`; rotation rule applied to the else-block stores.

## Remaining from the sweep (not attempted, with reasons)

- `func_00217860`, `func_002178C0`, `func_00217A08` — save `$ra` with
  `sq`, so blocked on the unsolved sq/lq question.
- `func_00217A60` — hinges on `movn` (recognise-and-move-on category).
  Worth noting though: its `movn` is the standard **signed-division-by-4**
  idiom (`addiu +3` / `movn` / `sra 2`), which plain C `x / 4` *does*
  generate — so this may not really belong to the "plain C won't produce
  movn" category. Untested; a cheap retry for someone.
- `func_0011EEC8` (core_text, 24 instrs) and `func_0011DFE8` (442
  instrs) — sign-extension prologue, no sq/lq, not reached this round.
  `func_0011EEC8` is the better next candidate.

## Verification state at end of round

Full sweep of every compiled function in both files, size and bytes:
**138 exact**, 3 size-mismatches (`func_00115098` -4, a documented
near-miss; `func_001F9B90`/`func_001F9B98` +4, the false-match class
above), 6 byte-mismatches (all previously documented near-misses:
`func_00115578`, `func_001160D8`, `func_00209048`, `func_002094E0`,
`func_0021B108`, `func_00234350`).

## Meta

"Tried everything" notes in the status table have now been wrong three
times: the sign-extension question itself, the `func_00113A70` drift
diagnosis, and the `func_0011AE1C` match. Re-test blocked entries when a
new technique lands.
