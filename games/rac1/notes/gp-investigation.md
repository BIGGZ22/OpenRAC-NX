# $gp / small-data-area (SDA) investigation

> **Historical round notes (September 2026), kept as a record.**
> `docs/DECOMP_PROGRESS.md` and `progress/report.json` hold the current
> state. Superseded since this was written:
> - Beyond the SOLVED banner below: the "blocked clusters" near the end
>   (one variable reached both via `$gp` and via `lui`/`$at` in the same
>   function) were the assembler macro form, solved by `MACRO_ADDR` (see
>   "SOLVED: the one-instruction macro form" in
>   `docs/DECOMP_PROGRESS.md`). `func_002094A8`, `func_002095E8`,
>   `func_00209358` and all five DMA packet writers (`func_00234C50`,
>   `func_00234E80`, `func_00234EE0`, `func_00234F40`, `func_00234FA8`)
>   are now exact.
> - `src/text.c` and `src/core_text.c` no longer exist: the source is
>   split into `src/core/` (one file per retail object) and `src/game/`
>   (one file per original source file).

# SOLVED. The answer was the -G threshold: use -G2.

Everything below this banner is the historical trail, and **two of its
conclusions were wrong** -- kept deliberately, because both were stated
confidently and someone re-reading them would otherwise repeat the dead
ends.

## The answer

Retail's small-data threshold sits in **1..3**; the build now uses
**`-G2`** (`Makefile.sn`). Measured behaviour of this compiler:

| `-G` | FP constants | small globals via `$gp` |
|---|---|---|
| `-G0` | inline (`lui`/`ori`/`mtc1`) | never -- so no `$gp` function can match |
| `-G1`..`-G3` | **inline** | **yes** |
| `-G4`+ | pooled into `.lit4` | yes |

Retail inlines FP constants *and* uses `$gp`, so it must be 1..3. At
`-G4`+ the pooling is not just wrong, it is unbuildable: `.lit4` is
addressed via `$gp` and the SDA window is fully occupied, so it has
nowhere to live.

**Proven end-to-end.** `func_001F6598` -- `sw $2, -0x7764($28)` -- is
byte-exact (0/12), the first `$gp` function ever matched here. Switching
to `-G2` cost nothing: 195 exact, zero regressions.

## The one trick you need

Placement is decided by an extern's **declared** size, and `-G2` only
admits variables of 1-2 bytes. Retail reaches plenty of *4-byte* globals
via `$gp`. Declare those as a small type and cast at the point of use:

```c
extern short D_0015F59C;              /* declared small -> lands in SDA */
*(int *)&D_0015F59C = 1;              /* accessed as the word it is */
```

`NOT_SDA` (in `include/common.h`) is the opposite lever, for variables
that must stay *out* -- needed for the ~60 that live outside the
window's +-32KB reach.

## Two wrong conclusions recorded below -- do not act on them

1. **"The literal pool needs somewhere to live."** It does not. Retail
   has no literal pool; at `-G2` neither do we. This was solving a
   problem created by choosing `-G8`.
2. **"Retail was built from multiple translation units with different
   `-G`."** Refuted twice over. It rested on treating inline-FP as proof
   of `-G0`, which is only true at `-G4`+. And measuring it directly
   found **13 pairs of immediately adjacent functions** that would need
   different `-G` -- impossible, since a linker lays each TU's `.text`
   out contiguously. A single `-G2` explains every observation with no
   split at all.

The lesson, which this project has now learned four separate times: a
confidently-worded conclusion in these notes is worth re-testing when
you have a new lever, because several have been wrong.

---

# Historical trail (superseded)


Status: **mechanism confirmed, blocker reduced to a defined task.** Not
yet applied — the build is still `-G0` and unaffected.

## What is settled

**Retail's gp base is `0x00166D00`.** Not inferred — read from the
binary's own ELF metadata, `.reginfo` -> `Elf32_RegInfo.ri_gp_value`,
and independently re-verified by parsing `.reginfo` from the baserom.
`_gp` is already defined in `rac1.ld.sh`, verified purely additive.

SDA window is therefore `0x15ED00 .. 0x16ED00` (gp ±32KB). All 1011
gp-relative references in the disassembly land inside it.

**Per-variable control works, and needs no use-site changes.** Confirmed
by direct experiment (`-O2 -G8`, v1.14):

| Declaration | Size | Codegen at `-G8` |
|---|---|---|
| `extern int x;` | 4 | `lw $v0, 0($gp)` — **gp-relative** |
| `extern char x[];` | incomplete | `lui` — stays out |
| `extern int x[64];` | 256 | `lui` — stays out |

At `-G0` everything is `lui`, i.e. no SDA at all — which is why no
gp-relative function can currently match.

So placement is decided by the *declared* size of each extern. Choosing
how to spell a declaration is enough to put a variable in or out of the
small-data area, without touching any code that uses it.

## What blocks a global `-G8` today

Switching the whole build to `-G8` compiles fine but fails to link, in
exactly two ways — both fixable, neither mysterious:

1. **60 externs wrongly become gp-relative** and their references don't
   reach: `relocation truncated to fit: R_MIPS_GPREL16`. They live far
   below the window (`0x12F86C`, `0x13Dxxx`...) while the window starts
   at `0x15ED00`. Full list: `notes/gp_offenders.txt`. Fix: declare each
   with an incomplete or oversized type per the table above.
2. **One section overlap**: `-G8` creates `.lit4`, which the linker
   script places at `0x1E8F00`, colliding with `.lvl_vtbl`. Fix: give
   `.lit4`/`.sdata`/`.sbss` explicit placement in `rac1.ld.sh`.

## Progress: two of the three obstacles are now solved

**1. The 60 wrongly-gp-relative externs: SOLVED.** `NOT_SDA` (in
`include/common.h`) is `__attribute__((section(".data")))`, which tells
the compiler the variable is not in `.sdata` so it falls back to
`lui`/`%lo`. Crucially this leaves every use site untouched, unlike the
incomplete-array trick, which matters because many of these are scalars
used inside already-matching functions. All 61 declarations are tagged.
Verified directly: a plain `extern unsigned char x;` compiles to
`lbu $v0,0($gp)` at `-G8`; the same declaration with `NOT_SDA` compiles
to `lui`/`lbu`. (`__attribute__((aligned))` does NOT work -- still
gp-relative.) With this applied, `-G8` produces **zero** GPREL16
truncations.

**2. Section placement for `.sdata`/`.sbss`: SOLVED.** They come out
empty, because our C defines no data of its own -- every global is
extern. Parked past the end of the image in `rac1.ld.sh`.

**3. The FP literal pool: NOT solved, and this is what still blocks
`-G8`.** At `-G8` the compiler puts float constants in `.lit4` and
addresses them via `$gp` (`R_MIPS_LITERAL`), so `.lit4` must live inside
the 0x15ED00..0x16ED00 window -- but **the window is completely full**:

    core.bss  0x154200..0x15ED7F   (runs right up to core.lit)
    core.lit  0x15ED80
    .lit      0x15F000
    .bss      0x161380
    .data     0x165580..

There is no free space, so there is nowhere to put a
compiler-generated literal pool. Currently only 12 bytes are needed
(3 float literals, all in `text.o`; `core_text.o` needs none).

The likely correct resolution, not yet attempted: retail evidently
compiled with a nonzero `-G` too, so **retail's `.lit`/`core.lit` ARE
its literal pools** -- our generated literals are duplicates of entries
already sitting at known addresses. The right fix is probably to make
our references resolve to retail's existing pool rather than emitting
our own, instead of finding somewhere to park a second copy. Placing
`.lit4` at our own address would make any function referencing a literal
differ from retail in that operand anyway, so parking it is not a real
fix even if space existed.

## The literal pool is NOT a placement problem -- it is structural

Investigated further; the conclusion changes the plan, so read this
before attempting the "suggested order of work" below (which is now
partly obsolete).

**Retail does not use an FP literal pool at all.** It materializes float
constants inline, e.g. `func_0021EF38`:

    lui  $1, 0x4049
    ori  $1, $1, 0x0FDB      /* 0x40490FDB = 3.14159265f */
    mtc1 $1, $f0

That is THREE instructions where a pool load would be one
(`lwc1 $f0, off($gp)`) -- retail chose the more expensive form despite
demonstrably having `$gp` set up. So there is nowhere to "place" our
`.lit4`: retail has no counterpart section for it to correspond to, and
any pooled reference is wrong regardless of where it lands.

**No flag and no sub-build separates the two behaviours.** At `-G8` this
compiler always pools FP constants AND uses `$gp` for data; at `-G0` it
does neither. Tried `-mno-embedded-data`, `-membedded-data`,
`-mno-gpopt`, `-msoft-float` -- none change it. All three available SN
sub-builds (v1.36, v1.14, v2.73a) pool identically at `-G8`, so unlike
the `sq`/`lq` question this is **not** a sub-build difference.

**What that implies.** A function that inlines an FP constant cannot
have been compiled at `-G8` by this compiler family, and a function
using `$gp` cannot have been compiled at `-G0`. Retail contains both.
Therefore retail was built from **multiple translation units with
different `-G` settings**, and our single monolithic `src/text.c` cannot
reproduce both at once -- `-G` is per-TU, not per-function.

Measured, consistent with this: 244 of 1036 `text` functions use `$gp`,
and they interleave with non-`$gp` functions throughout the segment
rather than forming clean blocks. (Interleaving alone doesn't prove
multiple TUs -- a single `-G8` TU also yields both, since only *small*
globals go via `$gp` -- but combined with the inline-FP evidence it
does: an inlining function and a `$gp` function cannot share a TU.)

**The real fix is architectural: split the sources per original
translation unit**, each compiled with its own `-G`, which is what
mature decomp projects do anyway (one `.c` per original TU). Since TU
boundaries are unknown, they must be inferred -- and there is a usable
heuristic: a function inlining an FP constant belongs to a `-G0` TU, a
function using `$gp` belongs to a `-G>0` TU. Placement would need either
contiguous per-TU address ranges or per-function
`__attribute__((section(".text.func_XXXXXXXX")))` with a generated
linker script.

That is a substantial change and should not be attempted piecemeal. The
`NOT_SDA` groundwork below remains correct and is already committed;
it will be needed whenever the split happens.

## Suggested order of work

1. Add `.sdata`/`.sbss`/`.lit4` output sections to `rac1.ld.sh`, placed
   inside the SDA window.
2. Re-spell the 60 offenders' declarations to keep them out of SDA.
   Check each use site still compiles — a scalar used as a scalar cannot
   simply become `char x[]` without a cast at the use site, and use
   sites inside already-exact functions must not change, so prefer an
   oversized complete type (e.g. a struct or `int x[64]`) where a cast
   would otherwise be needed.
3. Switch to `-G8` and sweep. Baseline to beat: **201 decompiled / 186
   exact / 0 size mismatch / 15 byte mismatch.** Any regression means a
   variable's placement is still wrong.
4. Only then attempt an actual gp-relative function.

## Caveat worth stating plainly

`-G8` is a guess at retail's threshold; 8 is the GCC default. It has not
been confirmed. If step 3 leaves residual mismatches concentrated in
gp-relative accesses, try other `-G` values before assuming the source
shape is wrong. Note the build already uses per-segment compilers, so a
per-segment `-G` is available if the two segments disagree.

## $gp harvest: decoded-but-unmatched (semantics banked)

Both reverted; the reading is certain, the residual is the known
allocator `%hi`-destination-reuse question, not source shape.

**`func_002348B8`** (9/44, 20%) — table lookup with a clamp:
```c
extern int D_0015EE84;
extern int D_001DE338[];
int i = D_0015EE84;
if (i >= 0x13) i = 0;          /* this is the movz */
*(int *)&D_0016100C = D_001DE338[i];   /* SDA store, gp -0x5CF4 */
```
Residual: retail loads `D_0015EE84` with `lui $2` / `lw $2,%lo($2)`
(reusing the destination as the address register); this compiler emits
`lui $3` / `lw $2,%lo($3)` and hoists both `lui`s to the top. Note
retail does it the *other* way in `func_001138A8` (separate register,
which we match), so this is genuinely allocation-dependent.

**`func_001EB300`** (22/56, 39%) — double indirection through two
globals:
```c
extern int D_0015F064, D_0015F060, D_001997FC;
extern short D_0015F780;       /* SDA, gp -0x7580 */
char *p = (char *)(D_0015F060 + *(int *)(D_0015F064 + arg0 * 4));
D_001997FC = *(int *)p;
*(int *)&D_0015F780 = (int)(p + 8);
```
Worth noting its store to the non-SDA `D_001997FC` uses a normal
register in retail (`sw $4,%lo(D_001997FC)($6)`), **not** `$at` — so the
global-STORE blocker is not universal: retail uses the `$at` macro form
only for standalone stores, and the split form when the address register
is already materialized for other reasons.

**Cost of the SDA collision here:** declaring `D_0016100C` as SDA (to
try `func_002348B8`) made the already-near-miss `func_00205220` 4 bytes
short. Since neither new function matched, the whole batch was reverted
rather than trading a real near-miss for nothing. The general rule from
the previous batch stands: only take the collision when it buys an exact
match.

**`func_00213C78`** (reverted; 4 bytes short, 41%) — walks a linked list
of objects, calling into each. Prologue matches retail instruction for
instruction; the divergence is inside the loop.
```c
extern char *func_0020E6B8(void);
extern void func_0020E3D0(void *);
extern void func_0020ED48(void *);
extern short D_00160024;               /* SDA, gp -0x6CDC */

char *e = func_0020E6B8();
*(int *)&D_00160024 = (int)e;
while (e != 0) {
    if (*(signed char *)(e + 0x20) >= 0) {
        void (*fn)(void *);
        if ((*(unsigned short *)(e + 0x34) & 0x40) == 0) func_0020E3D0(e);
        fn = *(void (**)(void *))(e + 0x74);
        if (fn != 0) fn(e);
        if ((*(unsigned short *)(e + 0x34) & 0x4) == 0) func_0020ED48(e);
    }
    e = *(char **)(e + 0x28);
}
```
Two things a future attempt should know. Retail stores the *raw return
value* to the global (`sw $2,-0x6CDC($28)`) while this shape stores the
copy in `$s0` — so the global assignment may want to come from the call
expression rather than from the local. And retail uses branch-likely
forms (`bltzl`, `bnel`) throughout the loop, with the pointer advance
`e = e->0x28` sitting in their delay slots; reproducing that scheduling
is where the 4-byte shortfall lives.

## $gp harvest round 2 (coordinator-directed)

**Candidate filter that predicts success.** Two same-size near-misses
(`func_001F45F0` 10/52, `func_0020DFF8` 9/68) were both stuck on the
allocator's *destination-reuse* choice, and that question only surfaces
when a `$gp` function ALSO loads a non-SDA global through a
`lw $x, %lo(sym)($x)` pair. Excluding that pattern -- on top of "no `$at`
store form" and "no variable needing both `$gp` and `%hi`" -- leaves 10
candidates, and the first two attempts from that list matched exactly.
Selecting candidates is a better lever here than iterating source shape.

**`func_00209290` (0/88) needed two techniques stacked**, neither
sufficient alone:
1. the rotation rule *did* apply (our build emitted the last source store
   first), so the seven `-1` stores were rotated to put `b+0x90` last;
2. retail hoists the slot base `b + 0xB0` into its own register before
   the stores -- computing it as an explicit early local rather than
   inline at the point of use took it from 25/88 to exact.

Note (2) is the same lever that worked for `func_00205728`, and (1) is a
counter-example to `func_00227A30`, where rotation made things worse. The
rotation rule is genuinely per-function; test it, don't assume.

**`func_001F0F78` -- decoded, reverted at 69%.** Ring-buffer append plus
a counter update. Semantics are certain:
```c
extern char D_0018AC00[];      /* 16-byte entries */
extern char D_0015F108[];
extern short D_0015F104;       /* SDA, gp -0x7BFC -- entry index */
extern short D_0015F100;       /* SDA, gp -0x7C00 -- running count */
extern int func_00116248(int, void *, int);

int idx = D_0015F104;
char *e = D_0018AC00 + idx * 16;
int n = D_0015F100;
e[0x0] = a0; e[0x4] = a1; e[0x8] = a2; e[0xC] = n;   /* all int stores */
D_0015F104 = idx + 1;
D_0015F100 += func_00116248(n, D_0015F108, a3) + 1;  /* reloads D_0015F100 */
```
The divergence is scheduling, not meaning: retail interleaves the two
SDA loads with the address arithmetic and the stack adjust (loading the
index into `$10` and the count into `$11` several instructions apart),
while this compiler emits both SDA loads back to back at the top. Retail
also recomputes the entry address into three different registers
(`$8`/`$9`/`$4`) across the four stores rather than reusing one. Not a
source-shape problem as far as I could tell.

**Blocked clusters confirmed (don't re-attempt without a new idea):**
- `func_002094A8` / `func_002095E8` / `func_00209358` -- same variable
  `D_0015EFB0` reached via `$gp` early and via `lui $at`/`%lo` later
  *within one function*. Verified this compiler never emits the `$at`
  macro form (it always allocates a normal register, e.g. `lui $v1`), so
  an aliased second symbol would not help either -- the difference is
  register *allocation*, not symbol placement.
- `func_00234C50` / `func_00234E80` / `func_00234EE0` / `func_00234F40` /
  `func_00234FA8` -- DMA packet writers with a clear `volatile` reload
  signature on the pointer `D_00161000`, but that pointer is loaded via
  `lui`/`%lo` and stored back via `$gp` in the same function: the same
  collision. Declaring it either way breaks the other half.

**New technique — don't put `++` inside the comparison.** `func_00234AC8`
(a timeout-wait loop) sat at 34/128 with exactly one surplus instruction:
`i++ > 100000` makes the compiler emit `move $v0,$s1` to hold the old
value before `slt`. Retail compares the counter register directly and
increments in the branch delay slot, which is what

```c
if (i > 100000) { ...; break; }
i++;
```

produces. Splitting the increment out of the condition took it from
34/128 to **0/128**. Worth trying wherever a near-miss is exactly one
instruction long inside a loop.

**A 32-bit constant built as `ori`/`dsll`/`ori` means the parameter is
64-bit.** `func_002208F8` passes `0x80FFA888` in `$8`. For an `int` this
compiler emits `lui`/`ori`; retail's `ori $8,$0,0x80FF` / `dsll $8,$8,16`
/ `ori $8,$8,0xA888` is the 64-bit build, which avoids sign-extending a
constant with bit 31 set. Declaring that parameter `long` and writing the
literal as `0x80FFA888L` reproduced it, and the function matched 0/164
first attempt. Useful tell: `dsll` in a constant build = the callee takes
a 64-bit argument there.

Also a reminder this function confirms: EABI passes the first eight
integer arguments in `$4`-`$11`, so six-argument calls are normal here
and `$8`/`$9` in a call setup are arguments, not scratch.
