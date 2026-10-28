# Ratchet & Clank 1 on Switch: experimental renderer build

Starting without arguments opens a controller-operated dashboard. A checks the
SD-card files, X opens the setup guide, Y shows port status, B returns home,
and + exits. This uses libnx's text console, not an in-game graphical HUD.
ZL starts the game with the SDL2/Mesa OpenGL renderer when the disc and the
exported title/first-level assets are present. B (the bottom Switch face button)
is PS2 Cross, A is Circle, X is Triangle, Y is Square, Plus is Start, and Minus
is Select. Both sticks and the shoulder buttons are mapped.

During gameplay, hold **ZL** for third-person lock-strafe: left stick moves
independently of Ratchet's facing and right stick turns the camera/aim. Release
ZL to return to normal movement. ZR keeps its original behavior. This is a
built-in console control extension, including when external mods are disabled.
It stays out of cutscenes, menus, vehicles and unsupported movement states.
Facing blends over a short 40 ms response time; grounded lateral movement uses
30 ms smoothing, or 45 ms when reversing. These are exponential response times,
not fixed delays. Neutral movement stops without added drift. Blending reads
actual motion each tick and runs before the game's collision handling; jumping
and special movement retain their own physics. Ratchet 1's existing animations
are retained, including the mod's backward walk/run cycle.

The implementation is adapted from OpenRAC/rac1-mods' GPL-3.0-or-later
[lock-strafe](https://github.com/OpenRAC/rac1-mods/tree/2c5139c106de03a7ae2d0ef55a703a1d7c290d09/lock-strafe),
pinned at `2c5139c106de03a7ae2d0ef55a703a1d7c290d09`. CMake fetches that source
and generates a ZL-only variant using the compact guest-memory accessors.

With valid disc files, R starts a headless boot test capped at 300 game frames.
It stops earlier if it reaches an untranslated function. This is a diagnostic,
not gameplay: no game picture is drawn. Hold L + R + Minus to exit at a game
frame boundary. Its session log is `data/openrac-rac1-pal.log` on the SD card.

**Experimental: basic gameplay has been reported working on a Switch.** The renderer, audio, input and
compact guest-memory implementations compile and link, and the compact address
translation passes PC tests. Complete playthroughs, movie-audio changes and
the latest strafe tuning still need broader hardware testing. Desktop debug panels are
omitted. Vulkan, enhanced lighting and FSR are disabled. Prefer Homebrew Menu
in full application mode: this loads level models and textures, not just the
small setup dashboard. Hold L + R + Minus to exit at a game frame boundary.
Explicit command-line arguments retain CLI use; `--window` enables rendering.

## Prepare the ISO on your PC

Do not mount the ISO. From the OpenRAC repository root in MSYS, run:

```sh
python tools/extractor.py "/c/path/to/Ratchet and Clank.iso" \
  --game rac1 --proj-path build/switch-data --extract --boot-only
python editor/extract.py port "/c/path/to/Ratchet and Clank.iso" \
  build/switch-data/port/pal --jobs 2
```

Replace the image path with your own. The current `rac1-pal` build needs the
supported PAL disc with executable `SCES_509.16`. The extractor validates the
disc and writes `build/switch-data/iso_data/rac1/`, including the full
`disc.iso`, the boot executable, `SYSTEM.CNF` and `buildinfo.json`.
`--boot-only` avoids extracting unused disc files; it still retains and
validates the complete ISO. Extraction does not build or launch the game.

Copy the prepared files to the SD card, keeping this layout:

```text
/switch/openrac/openrac-rac1-pal.nro
/switch/openrac/data/iso_data/rac1/disc.iso
/switch/openrac/data/iso_data/rac1/SCES_509.16
/switch/openrac/data/iso_data/rac1/SYSTEM.CNF
/switch/openrac/data/iso_data/rac1/buildinfo.json
/switch/openrac/data/port/pal/level_00/...
/switch/openrac/data/port/pal/level_99/...
```

Copy all exported `port/pal` level folders, not just the two examples. Without
them, the ISO and boot ELF alone are insufficient for the native world renderer.

The ISO must remain one file. If your SD card cannot store a file that large,
this build does not support split ISO files. The launcher checks ISO9660 and
ELF headers; complete disc validation happens in the PC extractor.

## Build and open the dashboard

From `port/`, with devkitPro's Switch toolchain installed:

```sh
pacman -S --needed switch-sdl2 switch-mesa
cmake -B build-switch \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Switch.cmake \
  -DSWITCH=ON -DOPENRAC_PORT_WINDOW=ON -DOPENRAC_GAME_IDS=rac1-pal \
  -DOPENRAC_VULKAN=OFF -DOPENRAC_ENHANCED_RENDERING=OFF
cmake --build build-switch --target openrac-rac1-pal_nro -j8
```

Copy `build-switch/openrac-rac1-pal.nro` to the location above and select it
in Homebrew Menu. The dashboard opens even before disc files are copied.
Use the NRO output; an ELF alone is not a Homebrew Menu application.

Compilation and packaging can be checked here. Controller behavior, display
and SD-card checks still need testing on actual Switch hardware.
