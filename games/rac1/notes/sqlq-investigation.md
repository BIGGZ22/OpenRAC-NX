# The `sq`/`lq` callee-saved spill question — resolved

> Historical log. `tools/fix_core_spills.py`, `tools/fix_tail_calls.py`, `tools/fix_trunc_slot.py` and `tools/func_cflags.py`, which it describes, were removed on 2026-10-07 for rewriting compiler output; see docs/BUILD_FIDELITY.md for the current build.

> **Historical round notes (September 2026), kept as a record.**
> `docs/DECOMP_PROGRESS.md` and `progress/report.json` hold the current
> state. Superseded since this was written:
> - Section 4's build change was adopted and then refined: `core_text` is
>   no longer compiled with v1.36 but with v1.14 plus
>   `tools/fix_core_spills.py`, because v1.36 mirrors retail's save-slot
>   layout.
> - Section 6 ("`core_text` functions needing s-registers ... stay
>   blocked") is resolved by that same change.
> - The measurements here were taken at `-O2 -G0`; the build uses `-G2`
>   now.
> - `src/text.c` and `src/core_text.c` no longer exist: the source is
>   split into `src/core/` (one file per retail object) and `src/game/`
>   (one file per original source file).

**Verdict: it is not a flag. It is two different original builds, and we
already have both compilers.** Confirmed end-to-end with byte-level
proof. The practical upshot is large, and part of it is that the
"`sq`/`lq` blocked" category — until now the single biggest blocker in
the project — was substantially a **misclassification**, not a real
blocker.

## 1. What retail actually does (the old premise was wrong)

`docs/DECOMP_PROGRESS.md` states "Retail always spills callee-saved
registers (`$ra`, `$s0`-`$s7`) as plain 64-bit doublewords (`sd`/`ld`)".
That was generalised from `core_text` and is **false for `text`**.
Measured across every disassembled function:

| Segment | s-reg saves via `sq` | via `sd` |
|---|---|---|
| `text` | **428** | 34 |
| `core_text` | 14 | **234** |

And for `$ra`: 520 `text` functions save it via `sq`.

The two segments are near-perfect inversions of each other. That is not
a per-function compiler decision — it is two builds.

## 2. What each available compiler does

Identical input, `-O2 -G0`, measured via `-S` (which sidesteps the
assembler that rejected `-mabi=n32` in earlier attempts):

| Sub-build | s-regs | `$ra` |
|---|---|---|
| v1.36 (`sn-prodg-3.01`, currently used for everything) | `sq` | **`sd`** |
| v2.73a (`sn-prodg-2.0`) | `sq` | **`sq`** |
| v2.74 (`sn-prodg-24`, `ee-gcc295.exe`) | `sq` | **`sq`** |
| v1.14 (`sn-prodg-24`, `ee-gcc2953.exe`) | `sq` | **`sq`** |

So v1.36 matches retail's `core_text` on `$ra`; the other three match
retail's `text` on `$ra`. All four emit `sq` for s-registers, which
matches `text` and not `core_text`.

## 3. Byte-level proof

`func_0023DFC0` (a lone-`$ra`-save function in `text`, retail uses
`sq`). Identical C compiled by each build; every instruction agrees
except the spill pair:

```
retail   f0ffbd27 0000bf7f <jal> 48008424 0000bf7b 0800e003 1000bd27
v1.36    f0ffbd27 0000bfff <jal> 48008424 0000bfdf 0800e003 1000bd27   <- sd/ld, 2 bytes off
v1.14    f0ffbd27 0000bf7f <jal> 48008424 0000bf7b 0800e003 1000bd27   <- sq/lq, exact
v2.74    f0ffbd27 0000bf7f <jal> 48008424 0000bf7b 0800e003 1000bd27   <- sq/lq, exact
```

Full build + link with `text.c` on v1.14 and `core_text.c` on v1.36:
`func_0023DFC0` verifies **1/28**, and the single remaining byte is the
`jal` target (`7c`→`7e`, i.e. the callee sits 8 bytes later in our
build) — the already-documented downstream drift category, not a codegen
difference. Its spill bytes match retail exactly.

## 4. Recommended build change (proven safe)

**Compile `src/text.c` with v1.14 (or v2.74) and `src/core_text.c` with
v1.36.**

```
V136=toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin/ee-gcc2953.exe   # core_text.c
V114=toolchain/sn-prodg-24/local/sce/ee/gcc/bin/ee-gcc2953.exe         # text.c
```

Verified safe: **all 119 currently-matched `text` functions are leaf
functions with no `$ra` or s-register spill at all**, so none of them can
regress under the switch. (That is also exactly why this went unnoticed
for 23 rounds — every `text` match so far was a leaf accessor, and the
948 spill-using `text` functions were skipped or reverted.)

I did **not** change `Makefile.sn` myself, since it changes the toolchain
everyone builds against — that's the coordinator's call.

## 5. The big consequence: a whole category was misclassified

A category breakdown of range 0x1E9080-0x1F9B00 recorded 95 of 165
remaining stubs (57%) as "`sq`/`lq` blocked". That range is in `text`,
where **retail uses `sq` for s-registers — the same thing our compiler
already emits.** Sampled range-A functions (`func_001E9088`,
`func_001E94E8`, `func_001E96B8`, `func_001E9808`, `func_001E99D8`,
`func_001E9EC8`) all use `sq` in retail.

Those functions were skipped because agents compared our `sq` output
against the docs' claim that retail uses `sd`, without checking retail's
actual bytes for that specific function. **They should be re-examined.**
For `text`, the only thing that was ever genuinely wrong is the `$ra`
direction, and v1.14 fixes that.

Corollary worth stating plainly: skip decisions in this project should be
based on the retail bytes for the specific function, never on a
segment-wide generalisation.

## 6. What remains genuinely blocked

**`core_text` functions needing s-registers: 234 of them.** Retail
spills those as `sd`; no available compiler emits `sd` for s-registers.
This is the real residual of the original question, and it is now much
narrower — one segment, one register class. No flag reaches it (see
below), so it would need a differently-built `cc1`, a binary patch, or a
fifth SN sub-build nobody has mirrored.

## 7. Flag search: exhaustive negative

- **`PRODG_MANUAL.pdf` is a dead end** — 39 pages, and it is the
  *debugger*/target-manager manual (windows, panes, register views). It
  documents no compiler options whatsoever. Extracted and searched in
  full; nothing about save/restore width, quadwords, or 128-bit
  preservation.
- **Extracted the complete `target_switches` table** out of `cc1.exe`
  (region `0x1953c0`-`0x195c78`) rather than grepping loose strings.
  Full option set: `mips16`/`entry`, `max-skip-*`/`align-*`, `ips`,
  `cpu=`, `abi=`, `debug`+`debuga`..`debugi`,
  `check-range-division`, `check-zero-division`, `5900`, `3900`, `4650`,
  `fix4300`, `mad`, `single-float`/`double-float`, `el`/`eb`,
  `embedded-data`, `embedded-pic`, `long-calls`, `half-pic`, `abicalls`,
  `gp32`/`gp64`, `fp32`/`fp64`, `hard-float`/`soft-float`,
  `mips-tfile`, `memcpy`, `stats`, `gpopt`/`gpOPT`, `rnames`,
  `gas`/`mips-as`, `split-addresses`, `long32`/`long64`/`int64`, and an
  SN-specific `vu0-use-vf0-<range>`.
- **Tested every plausible one in both directions** — including the
  standalone `-m5900` switch (distinct from `-mcpu=5900`; there is no
  `-mno-5900`, it is one-way), all `-mabi=` values, and all `-mips`
  levels via `-S`. **Nothing changes s-register spill width, and nothing
  makes v1.36 emit `sq` for `$ra`.** Neither direction is reachable by
  any flag this compiler exposes.

That exhaustiveness is what makes the two-builds conclusion solid rather
than an assumption: the behaviour is baked into each `cc1` binary, which
is consistent with the standing hypothesis that R5900 GPR TImode support
was a GCC *build-time* configuration choice.

## 8. Suggested next steps

1. Split the build per segment (§4) — cheap, proven, unblocks the `text`
   `$ra` functions.
2. Re-examine the range-A "`sq`/`lq` blocked" functions (§5); a large
   fraction are probably matchable as-is.
3. Correct the "retail always spills `sd`/`ld`" premise in
   `docs/DECOMP_PROGRESS.md` and re-audit every entry that cites the
   `sq`/`lq` question as its reason for being blocked.
4. `core_text` s-register functions (§6) stay blocked; that's the only
   part still wanting a compiler-level answer.
