# Build fidelity

What this build reproduces of the toolchain retail was built with, what it
models instead, and the rules that keep it that way. Read it before you add
anything to the build, and before you call a function matched.

## What "matched" means here

A function is matched when the C in `src/` builds, through `Makefile.sn`
(or `tools/try_func.py`, which runs the same steps), into the bytes retail
has at that function's address, and the whole image still links at retail's
addresses (`tools/build_sn.sh` audits both). Level code is checked the same
way per function by `tools/overlay_check.py`.

The build uses retail's own compilers:

| Code | Compiler | Why we know |
|---|---|---|
| game code (`src/game/`, most of `src/overlays/`), and the non-SDK `src/core/` objects | SN Systems ProDG GCC 2.95.3, build v1.14 (`ee-gcc2953.exe`) | its save-slot layout, small-data use and scheduling are retail's; see docs/TOOLCHAIN.md |
| Sony SDK objects (`ee29` in `config/core_text.objects`) and libgcc | Sony EE-GCC 2.9-ee-991111 (and 991111-01 for three libgcc modules) | functions match members of Sony's own `libgcc.a` and SDK archives byte for byte; 2.9-ee emits retail's `sd` saves and tail jumps natively |

Everything the compiler emits is assembled as it is, except for the
assembler and linker behaviours below. Options are given per source file,
the only granularity GCC 2.95 has.

## The rules

1. **No step may change what the compiler emitted.** Between the compiler
   and the object file, a tool may only do what retail's assembler or linker
   did to that code: pad, expand a macro, pick an encoding, place a section.
   Each such tool is listed under [What the build models](#what-the-build-models)
   with the evidence for it and how many functions depend on it.
2. **Flags apply to whole files.** `config/file_cflags.txt` gives a source
   file extra options. A function that needs options its neighbours do not
   was in another object in retail, so it goes in a file of its own; there
   are no per-function flags.
3. **A function that only matches through a new transform is not matched.**
   Leave it as a near miss (`nonmatching/`) and write down what retail did.
4. **New steps need evidence first.** A tool that models an assembler or
   linker behaviour goes on the list only with a section here: what the real
   tool does, how that was established (the real tool's output, or retail's
   bytes across many functions), and how many matches depend on it, measured
   as below. `tools/check_build_fidelity.py` holds the build to this list and
   runs in `tools/gen_progress_report.py --check`, so CI fails otherwise.

## What the build models

The real assembler for retail's game code was SN's `ps2eeas`. This build
assembles with GNU `as` (the compiler driver's), which differs in a few
places; the tools below make up those differences. The libgcc objects were
linked by SN's linker, which dead-strips differently from ours.

Numbers were measured on 2026-10-07, at 3,591 matched functions, the way
`tools/measure_build_step.py` does it: each tool replaced by a pass-through,
everything rebuilt in a separate checkout, and every C function checked
against retail on its own (layout effects excluded). "Depend" counts functions exact with
the tool and not without it.

| Tool | What it reproduces | Evidence | Functions that depend |
|---|---|---|---|
| `tools/ps2eeas_nops.py` | `ps2eeas` pads every loop shorter than six instructions before its backward branch (the R5900 short-loop erratum), separates an FP compare from a directly following `bc1`, and a `mtc1` from the instruction reading its register. The other way round, it leaves an `mfc1` next to the branch that reads its register, where GNU `as` puts a nop | measured on `ps2eeas` itself; retail text has 309 backward branches spanning exactly six instructions and 191 of 191 compare/branch pairs separated; a compiled `mfc1` then branch occurs four times, adjacent each time (func_L00_00269BE8, func_L00_002761C0) | 781 (1,592,084 bytes, 42.9% of the code) |
| `tools/ps2eeas_dli.py` | `ps2eeas`'s instruction sequence for each 64-bit constant (`dli`); GNU `as` picks other sequences for many values | the algorithm reproduces `ps2eeas`'s output on 871 constants | 113 (395,716 bytes, 10.7%) |
| `tools/check_macro_slots.py` | a one-instruction global access the compiler put in a branch delay slot comes out `$gp`-relative, as retail's toolchain assembled it; anything else in a slot fails the build | of 524 `$gp` accesses in compiled retail code to globals also reached through `lui`, 505 sit in a delay slot | 460 (1,398,252 bytes, 37.7%) |
| `tools/fix_orphan_hi.py` | the high half of a `%hi` whose `%lo` the optimiser removed, which retail's linker filled and ours resolves wrongly | retail has the right high half in such `lui`s (func_001E9808) | none by the per-function check, which masks relocated fields; the linked image needs it |
| `tools/fix_macro_load_delay.py` | the load-delay nop the assembler of `989snd.o` and `wad.o` put after a two-instruction macro load whose result is used next | retail's code in those objects (func_0012E060 has it, func_0012DDC0's one-instruction loads do not) | none of today's matches |
| `tools/fix_volatile_slot.py` | the delay slot of a call filled with the volatile store before it, which retail's assembler did for 2.9-ee objects and GNU `as` does not | retail (func_0012C990, func_00128F90) | 2 (720 bytes) |
| `tools/fix_jump_tables.py` | a switch's jump table placed at retail's table address (section placement only; no instruction changes) | retail's `jtbl_` symbols | 20 level functions (249,656 bytes) |
| `tools/strip_dead.py` | retail's linker removing unreferenced libgcc functions (the first `floor(size/8)*8` bytes go, a 4-mod-8 function leaves its last word) | reproduces every odd spot in retail's libgcc from GCC's own source (`src/libgcc/README.md`) | the libgcc objects `dp-bit.o`, `fp-bit.o` and `L__main` |

Together the assembler steps carry 1,078 functions, 1,710,356 bytes (46.1%
of the code); a function usually needs more than one of them. That is most
of the level code's long functions: almost every long function has at least one
short loop or FP compare. The padding changes no instruction the compiler
wrote; it adds the nops the real assembler adds (and keeps GNU `as` from
adding the one after `mfc1` that the real assembler does not), and SN's real assembler,
run on the same compiler output, gives the same code for 98.4% of the C
functions (below). Replacing these tools with it is the goal.

Steps outside the compiled code, for completeness: `tools/fix_denormal_floats.py`
and `tools/fix_vu0_macro.py` rewrite the retail assembly that
`tools/setup_asm.sh` generates (stubs and data) so our assembler reproduces
it; they never touch compiled C.

### Checked against the real `ps2eeas`

Both versions in the toolchain mirrors (1.9.6.516 from SDK 2.4, 1.9.25.758
from ProDG 3.01) assemble this build's compiler output, once one line is
left out. Every source file includes `include/labels.inc`, the GNU assembler
macros the retail-assembly stubs use (`include/include_asm.h`), and ps2eeas
cannot read them: that line, not the compiler's output, made both versions
overflow their stack in the earlier attempts (docs/DECOMP_PROGRESS.md, and
again on 2026-10-07 before the cause was found). rac3-uya-decomp, which
assembles some functions with ps2eeas, never gives it those macros.

`tools/check_ps2eeas.py` compiles every C file of the game code (the
executable's and the levels') as the build does, assembles the compiler's
output with ps2eeas itself, and compares each C function with the build's
object (2026-10-07, 1.9.6.516):

| Result | C functions |
|---|---:|
| Identical to the build | 2,280 |
| Identical once ps2eeas knows each extern's size before its first use | 372 |
| Different | 43 |
| Total | 2,695 |

So the steps above reproduce SN's own assembler on 98.4% of the C code. The
second row is ps2eeas's one pass: it only uses `$gp` for a symbol whose size
it has already seen, and GCC 2.95 writes extern sizes at the end of the
file, so as it is ps2eeas reaches those globals with `lui`/`%lo` and pads
the loads. Retail had the sizes before use (a definition earlier in the
same file, or an `.extern SYM, SIZE` line, as rac3-uya-decomp writes them;
Lombyte has its compiler declare small data early); the check assembles the
same input a second time with the compiler's own `.extern` lines at the top.
The 43 left are short-loop and FP padding in a few level files (levels 16
to 18 mostly), and `fastfunc.o`, which retail wrote by hand; with every size
known up front ps2eeas also uses `$gp` where retail used `lui`. ProDG 3.01's
1.9.25.758 gives the same counts once its divide padding is turned off as
Lombyte does (it overflows on 35 files otherwise, and retail has no padded
divides).

Moving the build to the real assembler is now concrete work rather than a
blocker: stubs without GNU macros (ps2eeas's own directives in place of
`glabel`/`endlabel`), the extern sizes before use for the 372, and the 43.
Until then the table above is how the build reproduces it, and
`check_ps2eeas.py` shows it does.

## Flags

GCC 2.95 has no per-function options (`__attribute__((optimize))` and
`#pragma GCC optimize` arrived in GCC 4.4). Retail could only build a
function with different options by building its object with them. So:

- every file is built with `-O2 -G2 -Iinclude -Wa,-I,.`;
- `config/file_cflags.txt` adds options to a whole file;
- a function that needs an option its neighbours do not sits in a file of
  its own, which is the honest statement that retail had it in another
  object. The level files under `src/overlays/` are groupings this project
  chose, not retail's objects, so splitting one changes nothing about the
  image; the executable's files follow retail's object boundaries as far as
  they are known.

What that meant for the nine functions that had per-function flags
(each file built whole with the flag, every C function checked):

| Function | Flag | Its file with the flag | Now |
|---|---|---|---|
| func_0023C960, func_0023C9B0 (`src/game/movie/disp.c`) | `-mno-split-addresses` | 2 of 2 exact | the file has the flag |
| func_L16_002D0328 (`src/overlays/l16_kalebo3/vendor_002A50F0.c`) | `-fno-sched-interblock` | 64 of 64 exact | the file has the flag |
| func_L15_002F9FF8 | `-fno-force-mem` | 2 of 3 | its own file, `vendor_002F9FF8.c` |
| func_L18_002FAEF8 | `-mno-split-addresses` | 7 of 12 | its own file, `vendor_002FAEF8.c` |
| func_L00_002E9D78 | `-fno-schedule-insns` | 12 of 53 | its own file, `shared/vendor_002E9D78.c` |
| func_L00_001ED6D8 | `-fno-force-mem` | 8 of 10 | its own file, `shared/camera_001ED6D8.c` |
| func_002282D0 (`src/game/pause.c`) | `-mno-split-addresses` | 43 of 124 | retail assembly: pause.c's boundaries are retail's object (from the NTSC split), so the flag cannot be the file's |
| func_0011CB40 (`src/core/00119D88.c`, `sceSifInitIopHeap`) | `-fno-schedule-insns` | 18 of 45 | retail assembly: the functions around it in Sony's IOP heap code break with the flag, so nothing shows it was an object of its own |

The two split-off shared files are new units of one function each in the
progress report; the level ones stay in their level's unit.

## Removed, 2026-10-07

Four steps changed what the compiler emitted, which no retail tool did.
They were removed after measuring what depended on them (each replaced by a
pass-through, every C function checked on its own):

| Step | What it did | Matched functions that depended on it | Now |
|---|---|---|---|
| `tools/fix_core_spills.py` | narrowed callee-saved `sq`/`lq` to `sd`/`ld` in core objects built with the game compiler | none: the objects that need `sd` are built with 2.9-ee, which emits it | removed |
| `tools/fix_tail_calls.py` (+ `tools/tail_call_functions.txt`) | turned a call-and-return into retail's bare `j target`, which GCC 2.95.3 cannot emit | one, func_0012DA28 (`_exit`, 8 bytes); the other retail tail jumps come out of 2.9-ee by themselves or are handwritten assembly | removed; func_0012DA28 is retail assembly again (it sits in crt0 between two handwritten functions) |
| `tools/fix_trunc_slot.py` | moved the last half of an int truncation into the return's delay slot | two, func_00119EA8 (144 bytes) and func_0012AAA8 (28 bytes) | removed; both are retail assembly again |
| `tools/func_cflags.py` (+ `config/func_cflags.txt`) | compiled single functions with other flags and spliced them into their file's output | nine (2,224 bytes) | removed; see [Flags](#flags) |

Twelve functions and 2,404 bytes depended on the three rewriters and the
per-function flags together, 0.06% of the code. Five of them are retail
assembly again (620 bytes: func_0012DA28, func_00119EA8, func_0012AAA8 and
the two under [Flags](#flags)); the other seven keep their flags as options
of a whole file.

## Other ways the C steers the compiler

These are in the source, not the build, and are listed here so nothing about
how matches are made is implicit:

- `MACRO_ADDR`, `NOT_SDA` and `SDATA()` (`include/common.h`) give an
  `extern` a section, which decides whether the compiler addresses it
  through `$gp`, a `lui` pair or an assembler macro. They are declarations:
  retail's own declarations had to give the same result.
- `qcopy()`, `qcopy_nc()` and `qzero()` (`include/common.h`) are retail's
  128-bit copies and zero stores, written as inline assembly in one header
  because GCC 2.95 has no C form for them. Matched functions call them;
  no function contains assembly of its own.
- Linker remnants and handwritten functions are retail assembly, counted as
  finished under docs/ASM_CLASSIFICATION.md; they are never presented as C.

A port to other platforms replaces all three; matching keeps them because
they are how this compiler reaches retail's bytes from plain C.

## Checking it

```
python3 tools/check_build_fidelity.py          # the build runs only documented steps
python3 tools/gen_progress_report.py --check   # includes the check above
bash tools/docker/run.sh bash tools/build_sn.sh  # from scratch, with the image audit
```

To check the assembler steps against SN's real assembler:

```
bash tools/docker/run.sh python tools/check_ps2eeas.py [--as=1.9.25|PATH] [SRC.c ...]
```

To measure what depends on a step (rule 4), or to re-check the numbers
above:

```
bash tools/docker/run.sh python tools/measure_build_step.py ps2eeas_nops
```

It builds HEAD twice in scratch worktrees, once with the step replaced by a
pass-through, checks every C function against retail on its own, and lists
the functions that are exact only with the step.
