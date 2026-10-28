# Ratchet & Clank 1 on Switch: experimental renderer build

For cloning the public Switch branch, prerequisites, installation and controls,
start with the [OpenRAC-NX README](../../../README.md).

Starting without arguments opens the controller-operated launch menu. L/R selects
a tab: Play, Display, Audio, Gameplay, Game fixes, Saves or Setup. Up/Down selects
a row and Left/Right or A changes it. Changes persist in
`sdmc:/switch/openrac/settings/rac1-pal.ini`. B returns to Play, X checks the
installation and save storage, and + exits. The launcher is graphical and supports touch as well as controller input.
A on the Play page, or ZL, starts the game once disc, assets and writable save
storage checks pass. The Saves tab reports write/close/read-back health and the
number of stored files under `/switch/openrac/memcard/SCES_509.16/`.
B in game is PS2 Cross, A is Circle, X is Triangle and Y is Square.

The Display page's **Launcher graphics** setting selects OpenGL or an
experimental native deko3d menu presenter. This switches the launcher only;
gameplay remains OpenGL. A deko3d gameplay backend is not implemented.
The deko3d presenter uses two native scanout images, GPU copy commands and
explicit queue completion before releasing or changing its resources. It uses
no GL context. The shared UI canvas draws custom cards and text from the
Switch's shared font; no system font file is distributed. An interrupted native
startup restores OpenGL on the next run. Both paths require console testing.

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

On the Play tab, with valid disc files, Y starts a headless boot test capped at 300 game frames.
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

## First-launch setup on Switch

Copy the NRO and a single supported PAL v2.00 ISO into `/switch/openrac/`.
Leave at least 2 GiB extra SD space and launch in full application mode.
Setup starts automatically when level data is missing or preparation was
interrupted. It verifies the ISO's size, boot identity and full SHA-1 against
the known PAL build, extracts the boot files, and creates all 21 scene caches.
The progress screen can be cancelled with +; completed levels are reused.
The Setup tab's A action also prepares, resumes or repairs the data.

The ISO stays in its original location. `data/iso_data/rac1/disc-path.txt`
records its path, used for game sector reads and level overlays. Do not move
or delete it after preparation. No second multi-gigabyte copy is made.

Each `data/port/pal/level_NN/scene.orcache` is written through a partial file,
flushed to SD, checksummed and published before its completion manifest.
A setup-in-progress marker prevents Play during an interrupted installation.
Versioned cache headers reject incompatible layouts; repair regenerates bad
caches. Saves and player settings are not reset. The cache format is internal
to this build and can be regenerated from the disc; do not distribute caches.

The same native installer has completed all scenes on PC, with textures,
class bounds/triangles and scenery counts compared to the PC export. Switch
installation speed and gameplay from these caches still need hardware testing.

## Optional: prepare the ISO on your PC

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

Compilation and packaging have been checked, and basic gameplay has been
reported working on a Switch. Broader testing of display, controller behavior,
movie audio and complete playthroughs is still needed.
