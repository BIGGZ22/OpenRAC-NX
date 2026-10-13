# openrac-rac1-ntsc: Ratchet & Clank (NTSC-U), native

The native program of Ratchet & Clank (NTSC-U, SCUS-97199), built from the
NTSC decomp in [games/rac1/ntsc](../../../games/rac1/ntsc) by
[hostgen](../../tools/hostgen/README.md). It was started from
[rac1-pal](../rac1-pal/README.md), the version that plays: what PAL's port
knows by address was carried to this version's addresses, not found again.

```sh
python3 tools/build_port.py rac1-ntsc                 # from the top of OpenRAC
port/build/release/openrac-extractor baserom/SCUS_971.99.iso --game rac1 \
    --proj-path <data> --extract --decompile          # the data folder, from your disc
port/build/release/openrac-rac1-ntsc --data <data> --cards <a folder> --frames 3000 --skip-movies
port/build/release/openrac-rac1-ntsc --data <data> --cards <a folder> --window --levels <levels>
```

`--data`, `--cards`, `--frames`, `--level`, `--skip-movies`, `--no-card`,
`--settings` and `--levels` are rac1-pal's. Without a card folder the game
shows its no-card warning and waits for a button, which only a window can
give (`OPENRAC_PRESS`, [port/README.md](../../README.md)).

## Where it is (2026-10-13)

| | |
|---|---|
| Builds and links | yes: 1,882 files of the NTSC decomp read, 10 left out (below) |
| Headless | the boot stage, the boot movie (skipped), the first sound bank, then the title loop for 3,000 frames with no function missing and no crash |
| Window, title | the logo and PRESS START are drawn and the camera, the five mobys and the hero's pose are read (the same values PAL's has); **the world behind them is not drawn and the frame is not cleared**, so the 2D layer smears. Not found yet why |
| Menus, new game, a level | not tried in a window. Before the title loop had C, the program fell through it into level 0's start: its program was copied in and relocated (737 functions, 1,284 global addresses) and it stopped in the level's set-up |
| Levels to draw from | none of its own: `editor/extract.py` reads the PAL disc only. The window test above drew from PAL's extracted levels |

It is not offered by the launcher yet (`play` is `planned` in
[launcher/actions.json](../../../launcher/actions.json)).

## What is carried, and how far to trust it

Everything below was written by [port/tools/versionmap](../../tools/versionmap/README.md)
from rac1-pal's files and the two discs' code. A function is `same` when
the two versions have the same instructions for it (only addresses differ),
`near` when it sits between the same neighbours but changed.

| File | From | |
|---|---|---|
| [libraries.tsv](libraries.tsv) | rac1-pal's | 251 rows; each note keeps PAL's symbol. `strcmp` and `strncmp` have no counterpart (NTSC-U's libc is laid out otherwise) |
| [hostgen.json](hostgen.json) | rac1-pal's lists | `relocate` from 0x15EF00 (every section from `core.lit` on is 0x100 under PAL's), the nine level hooks, `returns_64`, `host_functions` |
| [split_places.tsv](split_places.tsv) | rac1-pal's | 23 rows: only where the NTSC decomp's catalogue folds the same places into one name; where it names a place apart, the NTSC decomp's own function stands |
| [host/boot.c](host/boot.c) | by hand | the overlay hook, the relocation, the draw flags, the direct start |
| [host/hand.c](host/hand.c), [host/vector.c](host/vector.c) | rac1-pal's, read through | the hardware routines; the levels' copy of WadDecompress is bound apart (this catalogue lists it apart) |
| the other `host/*.c` | rac1-pal's, as carried | the native 2D and world hand-off, collision, variants. Each compiles against this version's prototypes; none is checked in play |
| [hand/*.c](hand) | rac1-pal's `hand/` | 76 functions the NTSC decomp has as assembly (one, the integer as a float, written here) |
| [hand/functional/](hand/functional) | the PAL decompilation's functional C and, for eight functions, its matched C | 228 functions the NTSC decomp has as assembly; 8 of them `near` (their header says so) |
| [drafts.tsv](drafts.tsv) | the NTSC decomp itself | 58 files whose own pending C (the `#else` of their `NON_MATCHING` guard) is read in place of the stub: where rac1-pal has nothing for the function, and for the six `near` functions that have a draft (the title loop is one). That decompilation never compiles these |

`options.c` (PAL's rows on the Video options page) is not carried: the page
it adds to is PAL's.

## Left out of the build

`hostgen.json`, "skip": files Clang cannot read as they are. Each is a stub
for all its functions.

| File | Clang says |
|---|---|
| `src/sdk/library/__divdi3.c`, `__muldi3.c`, `moddi3.c`, `udivdi3.c`, `umoddi3.c` | libgcc's macros with GCC-only inline assembly; the host compiler has its own |
| `src/sdk/library/malloc_extend_top.c` | assignment to a cast (`sbrk_base = brk`) |
| `src/overlays/l06/gameplay/entities/002b4770.c` | an asm label on a variable after its first use (line 1935) |
| `src/overlays/l13/gameplay/vendor/003058a8.c` | `D_L13_00174140` declared with two different struct types (lines 103, 854) |
| `src/overlays/l16/gameplay/entities/002a3f38.c` | `freeze_stats_score_ref_b` given two asm labels (lines 394, 861) |
| `src/overlays/shared/gameplay/vendor/002e9010.c` | `absolute_float` given two asm labels (the file's and `rnc/math/fast_math.h`'s) |
| `src/overlays/shared/ui/text/00262ae8.c` (its draft only) | the draft does not read |

Not carried, for the record: 9 of PAL's functional files that lean on the
PAL file they sit in beyond what the carry brings along, and 36 PAL
functions that are entries inside a function this decompilation cuts
otherwise.

## What is left

1. **The title's world.** Find why the world layers draw nothing here
   (`OPENRAC_DEBUG=1` logs the same camera and "layers drawn" as PAL's):
   compare the display-list hand-off (`host/render_native.c`) and the
   paging flags (`host/boot.c`) with PAL's run, frame by frame.
2. **Its own levels.** `editor/disc.py` accepts `SCES_509.16` only: NTSC-U
   needs its executable's name and size, its table of contents checked, the
   `gameplay_ntsc` lump in place of `gameplay_pal`, and the (cos, sin) table
   at 0x165500. Its output must go beside PAL's, not over it
   (`assets/port/rac1` is PAL's today).
3. **Play it.** New game and each level, with `--stop-on-missing`: every
   carried file is unproven until then. hostgen's report
   (`port/build/release/games/rac1-ntsc/gen/report.md`) lists 38
   functions still without C that the boot, title and level loops reach.
4. **The shared code's PAL addresses.** The viewer's address tables and two
   words in the front end have NTSC-U values now; `debug_menu.cpp` and
   `options_menu.cpp` are still PAL's and only act for `rac1-pal`. The
   vertical field-of-view factor (0.756) is PAL's.
