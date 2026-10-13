# The games

Each game version the port builds is a directory here, named as its
decompilation is under `games/` (`games/rac1/pal` is `rac1-pal`). The build
makes `openrac-<id>` for every one whose decompilation is present:

| Directory | Game | Decompilation | Entry (`main`) | Library table |
|---|---|---|---|---|
| [rac1-pal](rac1-pal/README.md) | Ratchet & Clank (PAL) | `games/rac1/pal` | `func_0012DB18` | complete, from [the survey](../../docs/port/RAC1_PAL_SURVEY.md) |
| rac1-ntsc | Ratchet & Clank (NTSC-U) | `games/rac1/ntsc` | not known yet | from its `config/us/symbol_addrs.txt` |
| rac2-ntsc | Going Commando (NTSC-U) | `games/rac2/ntsc` (`candidates/`, its rendered units) | not known yet | empty |
| rac3-ntsc | Up Your Arsenal (NTSC-U) | `games/rac3/ntsc` (without `i5bootn`, the online program; the menu, `frontbin`, as a group of its own) | not known yet | empty |
| rac4-ntsc | Deadlocked (NTSC-U) | `games/rac4/ntsc` | not known yet | its libc, from `config/libc.tsv` |

Everything else is shared: game memory and the guest contract
([runtime](../runtime)), the translator ([hostgen](../tools/hostgen/README.md)),
the library replacements and the program ([common](common)), the platform
layer and the renderer.

## What a game is

| File | |
|---|---|
| `hostgen.json` | what the game is and how to read its decompilation (below) |
| `libraries.tsv` | every library entry point the game calls that the port answers itself: symbol, library, name, port (`host`, `game`, `wrap`, `todo`), notes, optional signature. A `host` row binds the address to the shared replacement of that name ([common/libraries.tsv](common/libraries.tsv)) |
| `host/*.c` | only if the game needs C of its own: `wrap` functions, its overlay hook |

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
| `places` | where each level function sits in each level (rac1/pal's `functions.tsv`) |
| `groups` | parts of the tree that are separate programs over the same addresses (rac3's menu, `src/frontbin`, overlay 100) |
| `overlay_hook` | true if `host/` provides `openrac_game_loaded_overlay` (which level's program is loaded) |
| `hand`, `drafts` | C for functions the decompilation has as assembly: the port's own (`hand/hand.tsv`), and the decompilation's own drafts (`drafts.tsv`) ([hostgen](../tools/hostgen/README.md#a-games-configuration)) |
| `roots` | where hostgen's frontier report starts |

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
   as rac1-pal does ([rac1-pal/host/boot.c](rac1-pal/host/boot.c)).

A new game version is a new directory with these files; the build finds it.
