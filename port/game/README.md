# The games

Each game version the port builds is a directory here, named as its
decompilation is under `games/` (`games/rac1` is `rac1-pal`). The build
makes `openrac-<id>` for every one whose decompilation is present:

| Directory | Game | Decompilation | Entry (`main`) | Library table |
|---|---|---|---|---|
| [rac1-pal](rac1-pal/README.md) | Ratchet & Clank (PAL) | `games/rac1` | `func_0012DB18` | complete, from [the survey](../../docs/port/RAC1_PAL_SURVEY.md) |
| [rac1-ntsc](rac1-ntsc/README.md) | Ratchet & Clank (NTSC-U) | `games/rac1/ntsc` | `FUN_0012d9d8` | carried from rac1-pal's ([versionmap](../tools/versionmap/README.md)); runs into its title loop |
| [rac2-ntsc](rac2-ntsc/README.md) | Going Commando (NTSC-U) | `games/rac2/ntsc` (`src/`, with level placements from `config/levels/functions.tsv`) | `func_00131AE8` (its crt0, the port's own; `main` is `func_00131D98`) | from its symbol table and byte comparison with rac1-pal's and rac4's; reaches its main menu with a further-along decompilation |
| rac3-ntsc | Up Your Arsenal (NTSC-U) | `games/rac3/ntsc` (without `i5bootn`, the online program; the menu, `frontbin`, as a group of its own) | not known yet | empty |
| rac4-ntsc | Deadlocked (NTSC-U) | `games/rac4/ntsc` | not known yet | its libc, from `config/libc.tsv` |

Everything else is shared: game memory and the guest contract
([runtime](../runtime)), the translator ([hostgen](../tools/hostgen/README.md)),
the library replacements and the program ([common](common)), the platform
layer and the renderer.

The program in `common/` is being split into three folders, which the build
takes every `.cpp` file from ([Games.cmake](../cmake/Games.cmake)), so a
split adds no line to it. They are empty until their splits land:

| Folder | |
|---|---|
| `common/program/` | the program itself, in every game's program, with a window or without: `main`, its options, vsync, the host side of `game_host.h` (from `main.cpp`) |
| `common/frontend/` | the window, built only with one: the window, frame pacing, interpolation, world submission, input (from `frontend.cpp`) |
| `common/debug/` | the debug overlay's panels, built only with a window (from `debug_overlay.cpp`) |

## What a game is

| File | |
|---|---|
| `hostgen.json` | what the game is and how to read its decompilation (below) |
| `libraries.tsv` | every library entry point the game calls that the port answers itself: symbol, library, name, port (`host`, `game`, `wrap`, `todo`), notes, optional signature. A `host` row binds the address to the shared replacement of that name ([common/libraries.tsv](common/libraries.tsv)) |
| `host/*.c` | only if the game needs C of its own: `wrap` functions, its overlay hook. A game's versions share theirs (`"host_shared"`: rac1's in [rac1/host](rac1/README.md)), each with a table of its names and addresses. A routine several games have alike keeps one body in `common/lib/` and, in each game's `host/`, a two-line function at the game's address with the game's own facts: the WAD decompressor ([wad_routine.h](common/lib/wad_routine.h)) and the VU0 sine and cosine ([vu0_sine.h](common/lib/vu0_sine.h)) of rac2 and rac4 |
| `no_c.tsv` | every function hostgen has no C for, or wrote C for from its assembly, and whether the port needs C for it (below) |

`hostgen.json`:

| Key | |
|---|---|
| `id`, `title`, `game`, `serial` | `rac1-pal`; `Ratchet & Clank (PAL)`; the extractor's game (`rac1`); its executable on the disc |
| `source` | the decompilation, from the top of OpenRAC; `-DOPENRAC_<ID>_SOURCE=<dir>` builds another copy |
| `frame_rate` | 50 (PAL) or 60 |
| `entry` | the game's `main`, or null while it is not known: the program then says so and stops |
| `sources`, `skip`, `includes`, `defines` | the C to translate and how to read it |
| `names` | how the decompilation names code and data by address, as regular expressions with `addr` (and `overlay`) groups; rac1/pal's (`func_`, `func_Lnn_`, `D_`) by default |
| `symbols` | splat symbol files that give named globals and functions their addresses |
| `derived` | `{"build": ..., "table": ...}`: the names the decompilation's own build derives (Going Commando's `build/boot/placed.json`), read from that build when it is there, else from the port's copy beside the configuration; neither stops hostgen ([derived.py](../tools/hostgen/derived.py)) |
| `places` | where each level function sits in each level (rac1/pal's `functions.tsv`) |
| `groups` | parts of the tree that are separate programs over the same addresses (rac3's menu, `src/frontbin`, overlay 100) |
| `overlay_hook` | true if `host/` provides `openrac_game_loaded_overlay` (which level's program is loaded) |
| `hand`, `drafts` | C for functions the decompilation has as assembly: the port's own (`hand/hand.tsv`), and the decompilation's own drafts (`drafts.tsv`) ([hostgen](../tools/hostgen/README.md#a-games-configuration)) |
| `overrides` | the port's own C for functions the decompilation has as matching C, where the port has to do otherwise (`hand/overrides.tsv`): it replaces the matched definition in the port only, and the decompilation keeps its match ([hostgen](../tools/hostgen/README.md#a-games-configuration)) |
| `asm` | C from the assembly the game's setup wrote, for the functions with no C: its directories in the decompilation, `$gp`, whether it is on, and the retail program to read the same instructions from where the files are not (`"retail"`; a player's build reads its disc image) ([hostgen](../tools/hostgen/README.md#functions-with-no-c-c-from-their-assembly)) |
| `roots` | where hostgen's frontier report starts |

## The functions with no C

Each rac1 version's `no_c.tsv` lists every function hostgen has no C for, or
wrote C for from its assembly, and says whether the port needs C for it,
with the evidence: [no_c.py](../tools/hostgen/no_c.py) reads every call,
jump, branch, address formed in code and data word that refers to it in the
retail executable and in all 19 level programs, and hostgen's own call
graph. Class `a` needs no C, `b` is replaced by the host on purpose, `c`
needs C. As of 2026-10-18:

| | rac1-pal | rac1-ntsc |
|---|---:|---:|
| **a** needs no C | 475 | 1,432 |
| ... linker remnant (the decompilation's lists, `asm/remnants`) | 236 | |
| ... a piece of a function with C (only its branches and jumps reach it) | 143 | |
| ... padding (the linker's 0xCDCDCDCD fill) | 36 | |
| ... nothing refers to it in any program | 41 | 2 |
| ... another spelling of a function with C (calls go there) | 19 | 1,369 |
| ... declared, defined nowhere, called by nothing | | 61 |
| **b** replaced by the host on purpose | 27 | 3 |
| ... only host-replaced functions call it (a library's syscall wrapper, the WAD decompressor's helpers, VU0 animation code) | 12 | 1 |
| ... a piece of a host-replaced function | 11 | |
| ... the host's own (`fabsf`, `sqrtf`, `memcpy` as the host compiler's; the executable's crt0) | 4 | 2 |
| **c** needs C | 7 | 7 |
| ... of them with C now | 7 (translated from the retail program) | 7 (translated) |

Besides the table: libgcc's `__fixunsdfdi` (rac1-pal's `func_0011E6D8`, the
float printer's) and `__udivdi3` and `__umoddi3` (rac1-ntsc, newlib's
printf, strtol and malloc) are host rows of `libraries.tsv` now
([common/lib/libc.c](common/lib/libc.c)), so they are no longer stubs.

Every class `c` function has C: what hostgen translates from the retail
program, which a player's build reads from the disc image
([hostgen](../tools/hostgen/README.md#the-retail-program-where-the-assembly-is-not)).
rac1-pal's are the moby class callbacks `func_L00_00203ED8` and
`func_L00_0020DCB4` (data tables of every level), `func_L05_0023BAA0` (a
table of levels 5 and 16), `func_L00_002569F8` (called by the moby update
code), the MPEG colour conversion's `func_00127318` (code spimdisasm wrote as
data), the VU0 vector scale entry `func_00125328` and `func_00120C58`;
rac1-ntsc's are newlib's float conversion `func_00118338` (doubles in
general registers), the VU0 scale entry `func_001251E8`, the MPEG routines
`func_001271D8` and `func_0012C4C8`, the DMA completion handlers
`func_0012A2D8` and `func_0012A5D8` (handed to AddDmacHandler2), and newlib's
`malloc_extend_top`. None of the 509 and 1,442 was called in the regression
routes of all 19 levels on master (docs/port/REGRESSION.md, `reach`).

Rows whose evidence begins "by hand:" are judgements no_c.py keeps when it
runs again (two in rac1-pal: a padding word whose address + 4 is a handler
with C, and a hand-written piece whose address only looks like a level 3
data word).

## Moving a game on

1. Find the game's `main` and put it in `entry`; add its title and level
   loops to `roots`.
2. Build it and read `build/<preset>/games/<id>/gen/report.md`: what is
   translated, what became a stub and why, and the frontier (what the
   program reaches that has no C).
3. Bind the library functions it calls in `libraries.tsv` (the names in
   [common/libraries.tsv](common/libraries.tsv)); a library the shared code
   does not have yet gets a replacement in `common/lib/` and a row there.
4. If its levels load their own programs (overlays), give it an overlay hook
   as rac1 does ([rac1/host/boot.c](rac1/host/boot.c)).

A new game version is a new directory with these files; the build finds it.
