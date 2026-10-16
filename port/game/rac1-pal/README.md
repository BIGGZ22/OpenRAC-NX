# openrac-rac1-pal: Ratchet & Clank (PAL), native

The native program of Ratchet & Clank (PAL), built from the decompilation
in [games/rac1](../../../games/rac1) by
[hostgen](../../tools/hostgen/README.md). It is one of the port's games
([port/game](../README.md)): what is particular to it is here, the rest
(the library replacements, the program) is shared ([common](../common)).

```sh
cd port
cmake --preset release && cmake --build --preset release
build/release/openrac-rac1-pal --data <install>/active/rac1/data
```

`--data` is the folder the extractor wrote from the player's disc
([tools/extractor.py](../../../tools/extractor.py), or the launcher's "Set
up from your disc"): it holds `iso_data/rac1/SCES_509.16` and `disc.iso`.
The first memory card is the folder the launcher's save manager shows
(`~/.local/share/openrac/memcard/SCES_509.16`, or under `$XDG_DATA_HOME`;
`~/Library/Application Support/OpenRAC/memcard/SCES_509.16` on macOS) unless
`--cards` says otherwise; the game's save folder and files are ordinary files
in it. `--frames N` stops after N frames; `--keep-going` logs a
function without C and carries on instead of stopping at it. `--mods DIR`
plays levels edited in Godot: DIR is what `editor/extract.py pack` wrote, a
list of disc sectors (`patches.tsv`) and the files read in their place
([editor/README.md](../../../editor/README.md#pack-your-edits)).

To build another copy of the decompilation (your own checkout, further
along than OpenRAC's), configure with `-DOPENRAC_RAC1_PAL_SOURCE=<its directory>`.

## How it runs

1. Game memory is laid out as the console's ([port/runtime](../../runtime)).
2. The executable's loadable segments are copied to their addresses from
   the player's `SCES_509.16`: that is the game's data (its code bytes come
   along as data and are never run).
3. Every function is registered by code address; a level's functions are
   looked up in the level program that the game loaded last (`ParseBin`,
   [host/boot.c](host/boot.c)).
4. The game's own `main` (`func_0012DB18`, matched C) runs: the boot stage,
   then each level's loop.

**Where it stops today.** `main` calls the boot stage (`func_001E99D8`),
which is still assembly in OpenRAC's decompilation, so the program stops
there and says so. hostgen's report lists every function the program
reaches without C, nearest first (its "frontier"): with OpenRAC's
decompilation they are the boot stage, the title loop (`func_001EBB48`) and
the level loop (`func_L00_002465F8`).

## Files

| File | |
|---|---|
| [hostgen.json](hostgen.json) | the game and how hostgen reads it ([port/game](../README.md#what-a-game-is)) |
| [libraries.tsv](libraries.tsv) | every library entry point the game calls, and what the port does with it: most are bound to the shared replacements ([common/libraries.tsv](../common/libraries.tsv)) |
| [host/boot.c](host/boot.c) | the game's own host C: ParseBin wrapped, so a level code address means the loaded level's function (its overlay hook) |

The replacements themselves are shared by every game, by library, in
[common/lib](../common/lib): `cdvd.c` (the disc image), `mc.c` (memory
cards as folders), `graph.c` (frames, display, texture uploads, the display
list hand-off), `kernel.c` (interrupt handlers, no IOP), `pad.c`, `snd.c`
(silent for now), `libc.c`, `vu0.c`; the program is
[common/main.cpp](../common/main.cpp).

## What is not done

| | |
|---|---|
| Window and renderer | the program paces frames at 60 Hz (the PAL game is kept in its own 60 Hz mode) and logs what the game hands the hardware; the window ([port/platform](../../platform)) and the renderer ([port/renderer](../../renderer)) are to be connected at `openrac_game_vsync`, `openrac_game_dma_send`, `openrac_game_set_display` and `openrac_game_load_image` ([common/main.cpp](../common/main.cpp)) |
| Pads | what `scePad2Read` fills in is to be read from the game's own reader (`func_00217F68`) |
| Sound | 989snd is replaced at its API but silent |
| Movies | libmpeg is `todo`: the movie player is to be replaced above it |
| Readback | `sceGsExecStoreImage` (reading GS memory back) needs the renderer |
| `todo` rows | in [libraries.tsv](libraries.tsv) |
