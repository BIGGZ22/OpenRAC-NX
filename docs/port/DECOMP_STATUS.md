# How far the decompiled code is from a native build

What [games/rac1](../../games/rac1/README.md) holds today, counted
for the question the port asks: how much of the game is C that a PC compiler
could build, and what is not. Paths below are relative to `games/rac1`
unless they start at the repository root.

Counted on 2026-10-09 over this repository at `7954152` (rac1/pal synced
from rac1-decomp at `47f8556`, report of 2026-10-07), with one-off read-only
scripts over `progress/report.json`, `config/*.txt|tsv` and `src/**/*.c`,
using the regular expressions of `tools/gen_progress_report.py`. The disc-made
assembly (`asm/`) was not present, so everything here is from the source,
the configuration and the report. [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md)
measured some of the same things on 2026-10-08 at rac1-decomp `a35ffbee`;
where the two differ, this page is the newer count.

## 1. What "59.32 % matched" counts

The project's headline is bytes of code proven identical to the retail
program, out of 3,712,808 bytes: the boot executable's code (Sony's
libraries, newlib, libgcc and the game) and every distinct function of the
19 level programs, code shared between levels counted once.

`tools/gen_progress_report.py` counts as matched:

- a function whose C compiles to the retail bytes;
- a function listed in `config/handwritten_asm.txt` (`ASM_FUNC`): original
  hand-written assembly, kept as assembly and counted as finished, because
  no compiler produced it;
- the linker's filler words (`LINKER_REMNANT`, `config/linker_remnants.txt`).

Staged near misses (`nonmatching/`) only raise the "fuzzy" figure (60.99 %).

For a native build, only the first kind is portable C. The 2,202,320 matched
bytes split exactly into 2,113,788 bytes of C, 83,520 of hand-written
assembly and 5,012 of filler.

| Region | Code bytes | C | Hand-written asm | Filler | Near misses | Not decompiled |
|---|---:|---:|---:|---:|---:|---:|
| Sony libraries, newlib, 989snd (`core_text`, without libgcc) | 107,612 | 63.8 % | 9,008 (126 functions) | 904 | 124 | 28,928 (55 functions) |
| libgcc | 10,332 | 100 % | | | | |
| The game in the executable (`text`) | 347,756 | **40.6 %** | 74,512 (86 functions, 21.4 %) | 2,356 | | 129,832 (201 functions, 37.3 %) |
| **Boot executable** | 465,700 | 47.2 % | 83,520 | 3,260 | 124 | 158,760 (34.1 %) |
| Code shared by the levels | 1,200,628 | 45.2 % | | 1,480 | 24,272 | 632,712 |
| Code of one level only | 2,046,480 | 66.0 % | | 272 | 42,172 | 652,448 |
| **The 19 level programs** | 3,247,108 | **58.3 %** | | 1,752 | 66,444 | 1,285,160 (39.6 %) |
| **All** | 3,712,808 | **56.9 %** | 2.25 % | 0.13 % | 1.8 % | **38.9 %** |

1,519 functions are still `INCLUDE_ASM` stubs (55 in the libraries, 201 in
the executable's game code, 708 shared by the levels, 555 in one level),
about 1.44 MB. The function count (3,589 of 5,109) is 3,148 C functions,
212 hand-written ones and 229 filler entries.

**The game code alone** (the executable's `text` and all level code,
3,594,864 bytes) is 56.6 % C, 2.1 % hand-written assembly, 1.85 % near
misses and 39.4 % not decompiled.

## 2. Hand-written assembly: 212 functions to rewrite

These count as finished for matching and are all still assembly. The
classification is spimdisasm's "hand-written function" marker, so it is a
heuristic.

| Group | Functions | Bytes | What they are |
|---|---:|---:|---|
| Renderer cores | 52 | 59,692 | MobyProc and the rest of `mobyproc` (17 functions, 21,424 B; MobyProc `func_00212658` alone 5,152 B), ShrubProc and LightShrubs (8,100 B), TieProc and LightTies (7,804 B), TfragProc and LightTfrags (5,768 B), `drawquad` (5,872 B), `shadowproc` (4,512 B), PartProc (3,648 B), `skyproc` (2,564 B) |
| Collision | 4 | 9,528 | `func_001EE9F8` (4,984 B), `func_001EFE10` (4,336 B) |
| `fastfunc` vector and memory helpers | 19 | 1,940 | FastMemOr16, `sce_vu0_mul_matrix` and others (57 more `fastfunc` stubs, 2,648 B, are not decompiled; the file's comment says it was hand-written) |
| `miscproc` | 5 | 1,328 | InitDma, the WAD decompressor `func_0020C468` (scratchpad and DMA) |
| Interrupt handlers and others | 6 | | VIF1 DMA handler `func_00235118`, the movie's vblank handler `func_0023C7A8`, two in `mobyfunc`, one in `mobyutil`, one in `framebuf` |
| Sony's kernel library | 99 | 4,084 | 75 system-call stubs, thread and interrupt code, exception handlers |
| Sony's movie decoder (libmpeg, IPU) | 19 | 3,880 | MMI code |
| libvu0, crt0, small stubs | 8 | 1,044 | matrix routines, `_start`/`_exit` |

No level function is classified as hand-written, but 73 level functions use
the VU0 macro instructions and are still stubs: shared `mobyproc` is 4 % C,
the collision effects 7 %. No compiled C anywhere contains a COP2 (VU0)
instruction: all 171 functions that use them, and all 107 that use the
128-bit MMI instructions, are assembly or not decompiled.

## 3. Data: none of it is C

`config/splat.yaml` keeps every data section as assembly or binary, and
`rac1.ld.sh` says it: "our C defines no data of its own, every global is
extern."

| Section | Bytes |
|---|---:|
| `vutext` (VU0 and VU1 microcode) | 74,496 |
| `core_data`, `core_rdata`, `core_lit` | 151,296 |
| `lit`, `data`, the three vtbl records | 547,968 |
| `core_bss`, `bss` | 60,800 |

About 699 KB of initialised data for the executable, referenced from C as
945 distinct `D_XXXXXXXX` symbols; the level code references 2,905
`D_LNN_` symbols. **Each level's own data** (lit, bss, data and its moby,
camera and sound dispatch tables) is not in the repository at all: it exists
only in the level programs read from the disc (1.65 to 1.91 MB each, of
which 1.07 to 1.21 MB is code). The data holds absolute pointers: 506
function pointers in the executable's `data` alone, and the per-level
dispatch tables.

## 4. The level programs

Each of the 19 levels carries a full build of the game program, which
replaces the executable's main segment when the level loads (`ParseBin`
`func_0012DA38` copies the records; `main` calls the level's entry,
`func_L00_002465F8`, the level main loop, not decompiled, 2,248 bytes).
`config/overlays/functions.tsv` matches the copies: 743 level functions are
the executable's own code (their C is in `src/game`), 1,963 are shared by
several levels (1.197 MB), 1,468 belong to one level (2.05 MB, about 107 KB
per level). No level program is linked by the build yet
([OVERLAYS.md](../../games/rac1/docs/OVERLAYS.md), "Per-level rebuild").

C share of each level's own code: levels 00, 16 and 17 100 %; 18 84 %;
01 78 %; 15 74 %; 02 69 %; 03, 05, 06 about 64 %; 11, 12 60 %; 04 58 %;
08, 14 54 %; 07 41 %; 13 38 %; 10 29 %; 09 23 %; the shared code 45 %.

## 5. By subsystem

The executable's `src/game` files have real names. Share of each in C:

| Mostly C | Mixed | Mostly not C |
|---|---|---|
| camera 99 %, menu 100 %, hud 88 %, vuchain (the display list builder) 89 %, lights, stash, tiefunc, tfragfunc, initonce 100 %, help 79 %, music 78 %, mobyfunc 81 %, mobyutil 73 % | pause 53 %, draw 52 %, missionfunc 53 %, map 41 %, skyfunc 39 %, loaders 33 %, sound 31 %, memcard 30 % (its driver is not decompiled), pad 27 % | space (level streaming) 23 %, bmain (boot) 28 %, transition (title loop) 15 %, freeze (card dialogs) 6 %, update 3 %, the movie player 2-7 %, every renderer core and collision 0 % |

In the level code, the moby update functions (645 functions, 1.01 MB) are
32 % C by bytes, camera functions 27 %, sound functions 53 %. Of the hero's
code, the physics and state transitions are C; its initialisation, items,
lean, platforms and move pipeline are not. The level loader
(`func_L00_00244AE0`), the world draw (`DrawWorld` `func_001F3D78`), the
font printer and the sound update are not decompiled.

## 6. What must exist before the game's code can run natively

1. **C for the 1,519 functions still in assembly**, about 1.44 MB, among
   them the whole boot path and the frame: the boot stage `func_001E99D8`,
   the title loop `func_001EBB48`, the level loop `func_L00_002465F8`, the
   world draw `func_001F3D78`, level streaming `func_00204C60`.
2. **Rewrites of the 212 hand-written functions** (83.5 KB), and of the
   VU0 code hidden among the stubs (the 73 level functions, the `fastfunc`
   helpers, the VU0 microprograms FastSin and FastCos call).
3. **A native renderer** that does the work of the nine VU1 programs and the
   three VU0 microprogram images ([RENDERER.md](RENDERER.md)).
4. **The data as C**, or a loader that builds it from the player's disc:
   about 699 KB for the executable and 19 per-level sets that are not in the
   repository, with their pointers relocated ([PORTABILITY.md](PORTABILITY.md)).
5. **The level programs as one program**: the shared engine code once, and
   each level's own code and dispatch tables, which today all expect the
   same addresses.
6. **A platform layer** in place of Sony's libraries (section 7).
7. **The source made acceptable to a PC compiler** ([PORTABILITY.md](PORTABILITY.md)).

## 7. The platform surface

Everything Sony's is linked statically (`config/core_text.objects`). Share
of each library in C: newlib 67 %, the kernel library 59 % (99 hand-written
stubs), libgcc 100 %, libcdvd 54 % (`sceCdInit`, `sceCdDiskReady` and
`sceCdRead` not decompiled), libgraph 39 %, libdma about 97 %, libmc 99 %,
the pad, controller and vibration libraries 99 %, libvu0 0 %, libmpeg and
the IPU 61 %, libscf 99 %, 989snd 67 % (`snd_StartSoundSystem` and its RPC
layer not decompiled).

The game's C calls 101 distinct library functions directly:

| Library | Functions | Call sites in the executable's C | In the level C |
|---|---:|---:|---:|
| newlib | 8 | 32 | 105 (`rand` alone 99) |
| Kernel | 23 | 56 | 8 |
| libgraph | 10 | 33 | 6 |
| libcdvd | 6 | 16 | 1 |
| 989snd | 28 | 61 | 9 |
| libvu0 | 5 | 12 | 26 |
| Pad, controller, vibration | 7 | 7 | 0 |
| libmc | 4 | 5 | 0 |
| libdma | 3 | 4 | 0 |
| `sceGsResetPath`, `sceGsSyncPath` | 2 | 8 | 1 |
| libscf | 2 | 4 | 0 |
| WAD | 3 | 4 | 0 |
| libmpeg | 1 | 1 | 0 |

The undecompiled callers add more: about 42 989snd entry points in all,
`sceGsSyncV` at 36 sites, the 13 memory card calls (all inside
`func_00209E68`, which is not decompiled here; the survey listed it as C).
The console's second processor runs Sony's and 989's modules (sio2man,
mcman, mcserv, dbcman, sio2d, pdman, libsd, 989snd, the stash daemon); a
port replaces their services at the EE-side API, as OpenGOAL did
([OPENGOAL_NOTES.md](OPENGOAL_NOTES.md), section 7).
