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

## Where it is (2026-10-16)

| | |
|---|---|
| Builds and links | yes: 1,836 files of the NTSC decomp read, 10 left out (below) |
| Title | the title world, the logo, the menu and its dialogs; the menu's text sits near the top of the screen and its panels lower left (PAL's are centred together), and PRESS START was not seen |
| New game | title -> New Game -> "Continue anyway?" -> the opening captions -> Veldin's opening scene -> play, with no function without C called |
| Veldin (0) | plays: Ratchet walks, swings and throws the wrench, the follow camera frames him, the HelpDesk speaks; Start skips the opening scene |
| Novalis (1), Kerwan (3) | direct start (`OPENRAC_DIRECT=1 --level N`): the flight between planets (Start skips it), the arrival scene, then play. On Kerwan a particle callback (level 0's 0x267BE8) is called with no function |
| Other levels | not tried |
| Levels to draw from | its own, from the NTSC-U disc: the launcher's rac1/ntsc "Get the levels ready" writes `assets/port/rac1/ntsc` (`<data>/port/ntsc` in an installer); without `--levels` the program reads `<data>/port/ntsc`. The tests here passed `--levels ../../build/ntsc-levels`. Same format as PAL's; the manifests say `"version": "ntsc"` ([editor/README.md](../../../editor/README.md#the-two-discs) says how the two discs' levels differ) |

It is not offered by the launcher yet (`play` is `planned` in
[launcher/actions.json](../../../launcher/actions.json)).

## What is carried, and how far to trust it

Everything below was written by [port/tools/versionmap](../../tools/versionmap/README.md)
from rac1-pal's files and the two discs' code. A function is `same` when
the two versions have the same instructions for it (only addresses differ),
`near` when it sits between the same neighbours but changed.

| File | From | |
|---|---|---|
| [libraries.tsv](libraries.tsv) | rac1-pal's | 273 rows (carried again on 2026-10-16: the flight's update and the sky shells wrapped, the scene start written for this version, the checkpoint variants); each note keeps PAL's symbol. `strcmp` and `strncmp` have no counterpart (NTSC-U's libc is laid out otherwise) |
| [hostgen.json](hostgen.json) | rac1-pal's lists | `relocate` from 0x15EF00 (every section from `core.lit` on is 0x100 under PAL's), the eleven level hooks, `returns_64`, `host_functions` |
| [split_places.tsv](split_places.tsv) | rac1-pal's | 30 rows: only where the NTSC decomp's catalogue folds the same places into one name; where it names a place apart, the NTSC decomp's own function stands |
| [host/rac1_version.h](host/rac1_version.h) | the version map, checked by [hostnames.py](../../tools/versionmap/README.md#the-shared-host-cs-tables) | this version's names and addresses for the host C both discs share ([port/game/rac1](../rac1/README.md)): the overlay hook, the direct start, the hardware routines, the native 2D and world hand-off, collision, the HUD at a wide picture's edges, the Video options rows, the developer menu's Planets page. Each entry agrees with the map but those written above its mark with their reason |
| [host/scene_start.c](host/scene_start.c), [host/catalogue.c](host/catalogue.c) | by hand | what only this version needs: the in-game scene's start (hand C in PAL; here the NTSC decomp has no stub to replace) and the levels' copy of WadDecompress, which this catalogue lists apart |
| [hand/*.c](hand) | rac1-pal's `hand/` | 70 functions the NTSC decomp has as assembly (one, the integer as a float, written here) |
| [hand/functional/](hand/functional) | the PAL decompilation's functional C and its matched C (rac1-pal's `hand/` where it has its own) | 220 files for functions the NTSC decomp has only as assembly: refreshed on 2026-10-16 from what PAL had then (some `near`: their header says so), and 34 more on 2026-10-17 by [carry_functions.py](../../tools/versionmap/README.md#carrying-functions) (their header says so). The 97 rows (and files) that no longer replaced a stub, the NTSC decomp having its own C for them, are gone |
| [drafts.tsv](drafts.tsv) | the NTSC decomp itself | 58 files whose own pending C (the `#else` of their `NON_MATCHING` guard) is read in place of the stub: where rac1-pal has nothing for the function, and for the six `near` functions that have a draft (the title loop is one). That decompilation never compiles these |

The title's Options list gets the Video page back (the US game's list is
Language and Sound; the page is still in its data), with the port's rows on
it, as on the PAL disc.

## Left out of the build

`hostgen.json`, "skip": files Clang cannot read as they are. Each is a stub
for all its functions.

| File | Clang says |
|---|---|
| `src/sdk/library/__divdi3.c`, `__muldi3.c`, `moddi3.c`, `udivdi3.c`, `umoddi3.c` | libgcc's macros with GCC-only inline assembly; the host compiler has its own |
| `src/sdk/library/malloc_extend_top.c` | assignment to a cast (`sbrk_base = brk`) |
| `src/overlays/shared/ui/text/00262ae8.c` (its draft only) | the draft does not read |

Not carried, for the record: 9 of PAL's functional files that lean on the
PAL file they sit in beyond what the carry brings along, and 36 PAL
functions that are entries inside a function this decompilation cuts
otherwise.

## What is left

1. **The title's layout.** The menu's text sits near the top of the
   screen and its panels lower left (PAL's are centred together), and
   PRESS START was not seen. The menu works.
2. **Play every level.** Veldin, Novalis and Kerwan play; the others are
   not tried. hostgen's report lists 97 calls short of an argument
   (filled with 0): each is a call in the NTSC decomp's C that relies on
   a register the C does not set; fix them in its C as
   [OPENRAC_PATCHES.md](../../../games/rac1/ntsc/OPENRAC_PATCHES.md)
   records, worst first.
3. **The shared code's PAL addresses.** The viewer's address tables and two
   words in the front end have NTSC-U values now; `debug_menu.cpp` and
   `options_menu.cpp` are still PAL's and only act for `rac1-pal`. The
   vertical field-of-view factor (0.756) is PAL's.
