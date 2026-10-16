# Sibling decompilations

Other projects decompile or reimplement Ratchet & Clank games. Lombyte
and the NTSC decomp match the US build of this game, and the NTSC decomp has mapped per-file
compiler flags; ReRAC documents what the code does; rac3-uya-decomp
mapped retail's flags for a later game and works with this project. None is part of this build.

Inside OpenRAC, Lombyte is [games/rac1/ntsc](../../ntsc/README.md) and
rac3-uya-decomp is [games/rac3/ntsc](../../../rac3/ntsc/README.md); the tools find
Lombyte there. ReRAC is still cloned next to OpenRAC. In a standalone
checkout, clone them next to this repository:

```sh
git clone https://github.com/re-rac/rerac ~/Projects/rerac
git clone https://github.com/lombyte-project/Lombyte ~/Projects/Lombyte
git clone https://github.com/OpenRAC/rac3-uya-decomp ~/Projects/rac3-decomp
```

## Lombyte: the same game, US build

[Lombyte](https://github.com/mateuszklysz/Lombyte) (MIT) matches the US
executable, `SCUS_971.99`. Its code is ours, compiled for another
region, so a function it has matched is the best starting point we
have. It keeps one C file per function under `src/`, with names
recovered from the NTSC decomp.

Its percentage leaves out SIMD, VU0 and COP2 helpers ("intentional
asm"), so it reads higher than ours for about the same amount of
matched code.

`tools/lombyte.py` pairs its functions with ours:

```sh
python3 tools/lombyte.py map            # rebuild the pairing, executable and levels
python3 tools/lombyte.py func_001123A8  # -> _calloc_r, matched, its C file
python3 tools/lombyte.py _malloc_r      # a Lombyte name -> func_00114920
python3 tools/lombyte.py todo           # matched there, not here
```

It pairs through `config/overlays/us_map.tsv` when that exists: our map
of every US function to its PAL counterpart, made from the two builds'
code by `tools/overlays.py us-map` (docs/OVERLAYS.md, "US map"). A
Lombyte function's US address (`FUN_LNN_xxxxxxxx` in level NN, or its
place in the executable) gives our function, whatever their sizes. The
US binaries it compares are the ones ReRAC (ISC, below) extracts from
the US disc; the map itself is our own comparison. Without the map, and
for whatever it leaves, `map` falls back to aligning the two builds'
function sizes. Each pair records which (`"how"`), and Lombyte's size
when it differs from ours (`todo` shows it).

On 2026-09-27, 66 functions (31,576 bytes, 6.8% of our code) were
matched there and not here, the newlib allocator family and `_dtoa_r`
among them.

Lombyte decompiles level code too now, laid out as ours is
(`src/overlays/lNN/`, shared code named after level 00), so `map` also
pairs level functions, level by level. On 2026-09-30 it paired 3,409
functions; 291 (136,196 bytes) were matched there and not here. 73 of
them are the movie code upstream reverted to assembly (commit
`85ecd8b`, "Sources" in CONTRIBUTING.md): those are redone from the
assembly alone, never ported. The rest go out as queue waves
(QUEUE.md, "Lombyte ports"): a function's packet carries Lombyte's C
when Lombyte matched it. The first two such waves (lb1, lb2) ported
105 functions (63,424 bytes); `wave.py salvage --ports` later landed 7
more whose files had clashed (4,372 bytes). Each is listed in
THIRD_PARTY_NOTICES.md.

On 2026-10-02 the size alignment paired 3,028 functions, 186 of them
(75,812 bytes) matched there and not here. Through the map, `map` pairs
4,164 (all of them through the map), 276 (123,600 bytes) matched there
and not here: 91 more than before, and one size-aligned pair the map
showed to be a neighbour (func_L08_002F20D8 is Lombyte's
`FUN_L08_002f0c18`, not `FUN_L08_002f0b40`). In 58 of the 276 Lombyte's
function is another size than ours: the PAL code differs (an `aligned`
pair in the map) or the two projects cut the functions there differently.
A queue packet carrying such a port says so (`wave.py`'s `lombyte_port`).

### Porting a function

1. `python3 tools/lombyte.py func_X` gives the Lombyte C file. Start
   from its body; don't reinvent it.
2. Rename what it references:
   - **Functions**: a Lombyte name becomes ours with
     `tools/lombyte.py NAME`, or take the `func_X` from `CONTEXT.md`.
   - **Globals**: Lombyte's `D_XXXXXXXX` are US addresses. The PAL
     symbol is the one at the same place in our assembly
     (`asm/nonmatchings/<seg>/func_X.s`): the n-th `%hi`/`%lo` or
     `$gp` access there matches the n-th in theirs. `CONTEXT.md`'s
     globals list is in order of first use.
   - **Types**: `s32`, `u32` and `f32` exist in `include/common.h`.
3. Declare things the way this file already does. Keep Lombyte's
   control flow and statement order; they are what matched.
4. Credit it in the candidate's comment ("from Lombyte (MIT), adapted to
   PAL") and in the commit message. MIT also requires Lombyte's copyright
   and permission notice with any substantial copy: the first port adds a
   `THIRD_PARTY_NOTICES.md` carrying Lombyte's `LICENSE` ("Copyright (c)
   2026 Mateusz Kłysz"), and later ports list themselves there.

Lombyte builds some units with a patched EE-GCC 2.9 whose flags
reproduce codegen stock compilers lack (`sq`/`lq` saves, classic
`mult`/`mflo`, in-place `cvt.w.s`): see its
`docs/patched-toolchain.md`. A function that needed that profile there
may not match with our compilers.

Lombyte's newlib ports keep newlib's own macros, such as `MALLOC_ZERO`,
whose body is `do { ... } while (0)`. That is the original source, not
an artificial barrier, but `tools/integrate.py` refuses any `while (0)`,
so such a candidate is landed by hand after review.

## NTSC decomp: the same game, with per-file flags

The NTSC decomp matches the US boot ELF with
EE-GCC 2.95.2 (`-G8 -O2 -ffast-math -fno-exceptions`, SN's assembler
optional). Its hand-named `config/symbols.txt` is where
`config/symbol_names.txt` came from. Since 2026-09 an automated loop
matches functions there and names them; `decomp_state/matched.json`
lists 247 matched functions (2026-09-30), each with a note on what made
it match. Its names reach us through `tools/names.py` (docs/NAMES.md);
its C ports like Lombyte's (US addresses, `tools/lombyte.py` finds the
PAL counterpart).

What carries over most is its Makefile: per-object flags, each verified
against the whole NTSC boot image. Retail built some translation units
differently:

| NTSC decomp object | Flags |
|---|---|
| `menu`, `menu_post_mid`, `menu_post_gadgets` | `-fno-schedule-insns` |
| `menu_post`, `menu_post_pages`, `menu_post_pages_end`, `transition` | `-fno-schedule-insns -mno-split-addresses` |
| `menu_callbacks` | `-fno-schedule-insns2` |
| `pause_sched` | `-fno-schedule-insns` |
| `pause_post`, `pause_post2` | `-G0` |
| `movie/movie_mid`, `movie/videodec_post`, `movie/movie_post_audio`, `movie/videodec_nodata`, `movie/disp` | `-mno-split-addresses` |
| `permcb`, `vuchain`, `draw_post_reset` | `-mno-split-addresses` |

The NTSC decomp splits some of our units finer (`menu` into several objects), so a
flag applies to a range of functions, not necessarily our whole file.
Its notes (`decomp_state/notes/`) record what each matched function
needed.

**Measured here (2026-09-30): the flags do not carry over.** They are
relative to the NTSC decomp's compiler setup (EE-GCC 2.95.2, `-G8 -ffast-math`), not
to retail's objects as our SN 2.95.3 build sees them:

- Six exact `menu.c` functions inside the NTSC decomp's `menu` object (func_00207200,
  002072C0, 00207340, 00207648, 00207780, 00207930) under the NTSC decomp's
  `-fno-schedule-insns`: three stay exact, 00207200 goes to 14/188
  bytes off, 00207340 to 2/104, and 00207930 changes size.
- The one near-miss in that range, func_00227A70 (pause.c, inside the NTSC decomp's
  `pause_post2`, built there with `-G0`): 57/144 bytes off with default
  flags, `-G0`, `-fno-schedule-insns` and both; 62/144 with
  `-fno-schedule-insns2`; a size change with `-mno-split-addresses`. Its
  residual is source shape: retail keeps `%hi(D_001D5F70)` in `$t2`
  across the loop and forms the index with other registers.

So treat a flag from the NTSC decomp as a hint to test per function, never as a file
setting.

## ReRAC: the same game as a native PC port

[ReRAC](https://github.com/re-rac/rerac) (ISC) reimplements the game in
Rust from the US disc and Ghidra. It is not a decompilation, but its
design docs (`docs/plan/`, `docs/formats/`) describe what much of the
engine and level code does: the moby update mechanism and the per-class
update table (`moby_update_catalogue.md`), hero states, particles, HUD,
camera, collision queries. Its `tools/ghidra/names/doc_names.csv` names
the functions those docs discuss; the verified ones are in our
`include/names.h`, the rest are candidates in `config/names.tsv`.
Addresses there are US: the level programs' through Lombyte's overlay
catalogue (docs/NAMES.md).

Its extraction of the US disc (`extracted/boot/SCUS_971.99`,
`extracted/levels/NN/overlay.bin`) is what `tools/overlays.py us-map`
compares with our PAL code (docs/OVERLAYS.md, "US map"). Through that
map, `tools/overlays.py rerac-notes` quotes its function entries' names
and notes for our functions into `config/overlays/rerac_notes.tsv`
(crediting ReRAC and its commit), and `tools/dossier.py` puts them in
a worker's `CONTEXT.md` marked "ReRAC (ISC)".

## rac3-uya-decomp: Up Your Arsenal

[rac3-uya-decomp](https://github.com/OpenRAC/rac3-uya-decomp) (formerly
ratchet-uya-decomp) matches R&C 3's `frontbin.elf`, its main executable
`boot_elf.elf` and its launcher `i5bootn.elf` with the compiler our game
code uses, SN ee-gcc 2.95.3, and works directly with this project. Its
[compiler matrix](https://github.com/OpenRAC/rac3-uya-decomp/blob/main/docs/compiler_matrix_findings.md)
tested 15 compilers and 8 flag sets; the C patterns that carry over are in
[RAC3_PATTERNS.md](RAC3_PATTERNS.md). What else carries over:

- **Retail compiled some files with `-mno-split-addresses`.** Such a
  file loads a global with one assembler macro (`lw $v0, X`), so the
  compiler can't keep a `%hi` register across a call; the assembler
  expands a fresh `lui` each time. In a split file the `%hi` is shared.
  Files form address runs; UYA records them per range in
  `tools/text_parts.txt`. On func_001F3890, `-mno-split-addresses` gave
  retail's exact saved-register set, where every split-mode candidate
  needed two more. Our `MACRO_ADDR`, and giving one global two alias
  names, only imitate this for one variable at a time.
- Its other default flags are `-G8 -fopt-stack -mno-check-zero-division`
  (ours: `-G2`). A `div` without the zero-divide trap wants
  `-mno-check-zero-division`.
- Some of its functions are assembled with SN's own assembler (Ps2EeAs)
  for its `mtc1` hazard nops and inline float constants.
- Its `try_func.py` resolves relocations to real addresses instead of
  masking them, so two stores to different globals in the wrong order
  no longer pass. Ours does too since 2026-10-07, for executable
  functions (level functions always went through `tools/overlay_check.py`).
- Its `tools/regalloc.py` prints GCC's global allocation order; ours is
  `tools/regalloc.py func_X CANDIDATE.c`.
- **SN 2.95.3 never emits a sibling call** (`j func` in place of `jal`
  and `jr $ra`); Sony's 2.9-ee-991111 does. Its launcher matrix showed
  it, and our `tools/rank_candidates.py` blocks a tail jump outside the
  `ee29` objects for the same reason.
- It counts hand-written functions and linker remnants as done: they live
  in `asm/handwritten/` and `asm/remnants/`, included with `ASM_FUNC` /
  `LINKER_REMNANT`. We adopted that reporting policy for the 212 confirmed
  handwritten functions and 69 dead-strip remnants (83,796 bytes) listed
  in `config/`. Fragment buckets remain unmatched pending boundary fixes.
  The full build audit still counts exact C matches separately; see
  `docs/ASM_CLASSIFICATION.md`.
- It writes VU0 functions as C with inline asm, as their original source
  was. That doesn't carry over to us: our large VU functions (51
  functions, 67K bytes) have no gcc stack frame and use trapping
  `add`/`sub`, so they were hand-written assembly. At most about 45 small
  VU0/SIMD functions (3-5K bytes) could fit that pattern.
- It puts retail's extra padding after a function (more zero words than
  gcc's alignment adds) into the source ahead of time, with a
  `TEXT_PADDING(N)` macro (`tools/trailing_padding.py`), so converting the
  function to C needs no special step. Here workers still emit those nops
  themselves after the function (LEVERS.md). Counted on 2026-10-07, only
  three open executable stubs have such padding (func_0023B210,
  func_001F9F30, func_0020CDE0), and level functions are placed one at a
  time, so the setup step it would need is not worth adding.

Try a flag on one candidate with `TRY_CFLAGS`, set inside the container:

```sh
bash tools/docker/run.sh sh -c \
  "TRY_CFLAGS=-mno-split-addresses python tools/try_func.py func_X build-sn/try/func_X/pN.c"
```

The build still uses one set of flags for all game code. Which of our
files need which flags is not mapped yet.

### UYA's flags measured on this build (2026-09-27)

- `-fopt-stack` and `-mno-check-zero-division` don't apply: our retail
  saves `$s` registers with `sq` in 16-byte slots, and its `div`s carry
  the `break 7` trap, which is what the default flags produce.
- `-mno-split-addresses` and `-G8` were run over seven candidates whose
  residuals involve address formation or registers (func_001FF958,
  func_00213C78, func_00227A70, func_00200248, func_00201A38, plus the
  exact func_0020D960 as a control). Neither flag fixed any of them.
  `-mno-split-addresses` changed the size of three and broke the exact
  control (47 of 120 bytes differ); `-G8` only fails to compile where a
  declaration relies on `-G2` small-data placement. So hud.c, mobyutil.c,
  pause.c and mobyfunc.c are split-address, `-G2` files as built. The
  flag stays a per-function experiment for other files.
- Both SN assemblers UYA uses are in our toolchain mirrors
  (`sn-prodg-3.01/.../ee/bin/Ps2EeAs.exe`, `sn-prodg-24/.../ee/bin/ps2eeas.exe`).
  The earlier attempt to assemble the text segment with them failed
  because every file includes `include/labels.inc`, GNU assembler macros
  ps2eeas cannot read (it overflows its stack); UYA never feeds it those.
  Without that line ps2eeas assembles our compiler output, and
  `tools/check_ps2eeas.py` compares it with the build function by function
  (docs/BUILD_FIDELITY.md, "Checked against the real assembler").

### Measured on this build on 2026-10-07

- **`-mvu0-use-vf0-vf2`**, which UYA builds everything with because the
  register range it adds moves loop.c's hoisting thresholds: on all our
  level code it breaks 19 of 2,056 matched functions and makes none of
  410 near misses exact, so retail RAC1 was not built with it.
- **Its 23 functions with identical code here**: 16 are VU0 inline
  assembly in UYA (not allowed here), 7 are the movie player's buffers
  (never ported, docs/MOVIE.md). None came over.
- **`_exit`** (func_0012DA28, a bare `j exit` with `$a0` cleared):
  UYA's launcher has an `_exit` that matches as 2.9-ee C, but ours sits
  between the two handwritten functions of `crt0`, an assembly object, so
  it is assembly too; marking it handwritten is upstream's call
  (docs/ASM_CLASSIFICATION.md).
- **Its short-loop padding fixes** (counting `jal`, loops with a `div`, a
  `$gp` load in a poll loop) correct a filter that counts lines of the
  compiler's output; `tools/ps2eeas_nops.py` measures loops in the
  assembled object, so those cases were never miscounted here.

### `nop; nop` before `div.s`, `sqrt.s` and `rsqrt.s` (2026-09-30)

UYA's open problem (two `nop`s in front of most float divides and square
roots; none of its compilers or assemblers adds them) is a compiler
feature, not an assembler one. Sony's 2.96-ee-001003-1 compiler, the Linux
`cc1` under `toolchain/sn-prodg-24/local/sce/ee/gcc/lib/gcc-lib/ee/2.96-ee-001003-1/`,
has two output templates for each of the three instructions:

```
div.s   %0,%1,%2
%(nop\n\tnop\n\tdiv.s\t%0,%1,%2%)      (inside .set noreorder)
```

- The padded one is its default: a test file compiled with `-O2` or `-O0`
  gets `nop; nop; div.s` for every divide (nine of nine). A divide in a
  branch delay slot was not tested.
- `-mno-handle-ee-div-pipeline-bug` selects the plain one. The flag is
  the workaround for the EE's divide pipeline bug.
- SN's 2.95.2/2.95.3 and Sony's 2.9-ee-991111 have only the plain
  template and no such flag, so no option makes them pad.
- Retail Ratchet & Clank 1 (PAL) has no padded divide among 1,671 (the few
  single `nop`s are the usual one after `mtc1`), so this does not apply
  here.

Not explained yet: UYA functions that mix two, one and no `nop`s.

## rac2-decomp: Going Commando & Deadlocked Cross-Game Intelligence

[rac2-decomp](https://github.com/llesieur99/rac2-decomp) targets the US
executable of *Ratchet & Clank: Going Commando* (`SCUS_972.68`). Because
Insomniac Games developed the PS2 games under intense yearly release cycles
(2002 to 2005) on the same core engine architecture, extensive code, algorithms,
and structures were reused across games:

### 1. Byte-Identical Engine Functions (24+ functions shared)
`rac2-decomp` verified that at least **24 reviewed C functions** are
byte-identical between RAC1 and RAC2:
- `FUN_0028B740` (HUD alignment / bounding box calculation, matching `FUN_L00_00235a70` in `gameplay_animation_00235878.c`)
- `FUN_002A7AA8` (cubic spline / Hermite interpolation, matching `FUN_L00_00257e20` in `gameplay_entities_00257d78.c`)
- `FUN_002A8860` (integer pack/unpack manipulation, matching `FUN_L00_00259430` in `math_interpolation_00257ef0.c`)
- `FUN_002A8AF0` (2D point-in-polygon ray-casting test, matching `FUN_L00_00259740` in `gameplay_entities_00259710.c`)
- `FUN_002AA140` (`lerp(a, b, t)`: `a + (b - a) * t`, matching `FUN_L00_0025b6a8` in `gameplay_entities_00259710.c`)
- `FUN_002AAF40` (3-way element swap/permute by bitmask, matching `FUN_L00_0025c088` in `unclassified_0025bb38.c`)
- `FUN_002CC6A0` (clearing state flag at `+0x44`, matching `FUN_L00_00277f60`)
- `FUN_00312B58` (vendor moby state check `moby->state == 6`, matching `FUN_L16_002c4710`)
- `FUN_00312E10` (byte flag check at `+0x20`, matching `FUN_L00_002d8128`)
- `FUN_003505E0` (Ring buffer FIFO consumption at `base + 0x50000`, matching `FUN_L00_002ef300` in `runtime_buffers_002ef300.c`)

### 2. Shared Enums & Systems
- **RaC1 Gadget Enum**: 29 gadgets (`GADGET_BOMB_GLOVE` = 0 through `GADGET_PERSUADER` = 28)
  reused directly by RaC2's save-import system.
- **Memory Card FSM (`CardState`)**: Identical 25-state machine (`CS_INIT` to `CS_PROMPT_BEGIN_NOSAVE`).
- **Geometry Pillars**: `tfrag`, `tie`, `shrub`, `moby` collision pill sweep
  `MB_CheckCollPill` and camera collision `Camera_CollPrimTest`.

## Wrench: modding suite and engine asset definitions

[Wrench](https://github.com/chaoticgd/wrench) (GPL-3.0-or-later) by chaoticgd
and contributors is a modding toolkit and level editor for the PS2 Ratchet &
Clank games. It is not a decompilation, but it is the original source of our
asset format knowledge and provides reverse-engineered C++ type definitions
for moby instance private variables (`pvars`).

What carries over:
- **Moby PVar structures**: In `data/overlay/src/game_rac/`, Wrench defines
  the layout of private state variables (`pvars`) for ~20 moby classes (Plumber,
  Skid McMarx, Big Al, Novalis lift, etc.) and shared NPC dialogue systems
  (`npcVars`, `npcStep`, `npcstring`). These are adapted into
  [`include/moby_pvars.h`](../include/moby_pvars.h) and documented in
  [`docs/PVARS.md`](PVARS.md).
- **Asset and level formats**: Level data headers, WAD compression, terrain
  (`tfrag`), ties, shrubs, sky, and occlusion bounds ([`docs/ASSETS.md`](ASSETS.md)).
- **Moby class catalogue**: Class IDs and naming schemes
  (`tools/extract/moby_classes.tsv`).

