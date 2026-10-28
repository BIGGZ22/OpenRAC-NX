# OpenRAC-NX

**Ratchet & Clank 1 on Nintendo Switch, built natively from OpenRAC's decompiled code.**

OpenRAC-NX is an experimental Switch fork of [OpenRAC](https://github.com/OpenRAC/OpenRAC).
It rebuilds the game for Switch through devkitPro/libnx and an SDL2/Mesa OpenGL
frontend. It does not run a PS2 emulator.

This fork currently focuses on **Ratchet & Clank 1 PAL, `SCES_509.16`**. The
Europe/Australia v2.00 disc has been validated during development. Other games
and disc versions are not the target of this Switch build.

## Availability and current status

The source is public: **anyone can clone this branch and build it** with the
dependencies below. The Switch changes are on **`codex/switch-port`**; use that
branch when cloning. A build on a completely fresh computer has not yet been
verified.

Basic gameplay has been reported working on a Switch. This is an early port:
complete playthroughs, performance across levels, and the latest movie-audio
and strafe changes still need more console testing.

Included features:

- First-launch ISO validation and asset preparation on the Switch, with resumable level caches.
- A graphical controller/touch launcher, saved settings and visual setup progress.
- An OpenGL game frontend and an experimental deko3d launcher presenter.
- Gamepad input, audio output and compact guest-memory backing.
- Corrected full-tilt diagonal stick movement.
- Movie/gameplay audio-device handoff for full-screen cutscenes.
- Hold-ZL third-person lock-strafe, with facing and movement smoothing.

Desktop debug panels, Vulkan, enhanced lighting and FSR are disabled in this
Switch build. The original Ratchet 1 animations are retained.

**No ISO, PS2 executable, extracted models, textures, sounds or prepared game
data is included. You must provide your own supported disc image.** The NRO
needs your supported ISO. This build can prepare its boot files and renderer caches directly on the Switch.

## Install a released NRO: no PC extraction needed

1. Copy `openrac-rac1-pal.nro` into `SD:/switch/openrac/`.
2. Copy your own **Ratchet & Clank 1 PAL Europe/Australia v2.00 ISO** into the
   same folder. For example, name it `Ratchet.iso`. Keep only one ISO in that
   folder so setup can choose it unambiguously. The older installed location
   `data/iso_data/rac1/disc.iso` is also supported.
3. Leave **at least 2 GiB of additional free SD space** for generated caches
   and working space. The full-disc PC test generated about 936 MiB of caches;
   setup checks available space before writing each level.
4. Launch OpenRAC through Homebrew Menu in **full application mode**.
5. First-launch setup validates the complete ISO, extracts the boot files and
   prepares all 21 renderer scenes. A progress screen shows its current step.
   Keep the console powered and allow time for the initial setup.
6. When setup completes, press **A** on the Play tab to start.

```text
SD:/switch/openrac/
  openrac-rac1-pal.nro
  Ratchet.iso                 <- your supported disc image
  data/                       <- created by the application
```

**Leave the ISO in place:** gameplay still reads sectors from it. Setup stores
its path and does not make a second copy. The boot files and versioned renderer
caches are created under `data/iso_data/rac1/` and `data/port/pal/`. Saves remain
under `memcard/SCES_509.16/` and settings under `settings/`.

Press **+** during setup to cancel. Completed, checksum-verified level caches
are kept and reused on the next attempt; incomplete files are not accepted as
finished levels. In the **Setup** tab, press **A** to prepare, resume or repair.
If you run out of space, free space and restart. Existing saves are preserved.
Setup errors are also recorded in `data/setup.log`; review logs for personal paths before sharing them.
PC-prepared installations continue to work.

This installation path has been compiled for Switch and exercised on a PC
against the supported disc, including comparison with the PC exporter. Its
first-launch performance and gameplay with generated caches still need console
testing. Report any setup error with the step shown on screen.

## Building from source (optional for release users)

- A homebrew-capable Nintendo Switch with Homebrew Menu, preferably running
  in full application mode for the memory needed by levels and textures.
- A PC if building the NRO yourself or using the optional PC extraction route.
- Your own supported Ratchet & Clank 1 PAL ISO.
- [devkitPro](https://devkitpro.org/) with devkitA64, libnx and Switch tools.
- The `switch-sdl2` and `switch-mesa` portlibs.
- CMake 3.24 or newer, Make, Git, Python 3, and a **host Clang compiler**.
  Clang reads the decompiled game sources; devkitA64's GCC compiles the Switch
  program. Both are needed.

The commands below use an MSYS/devkitPro shell with the devkitPro package
repositories configured. Keep configuration and builds in the same MSYS
installation. Install the Switch dependencies there:

```sh
pacman -S --needed switch-dev switch-sdl2 switch-mesa
```

Install the host tools above as well. On an MSYS2 installation, host Clang can
be installed with `pacman -S --needed mingw-w64-x86_64-clang`; the Switch build
looks in `/mingw64/bin` and `/clang64/bin` as well as the normal tool search path.

## 1. Get the source

```sh
git clone --single-branch --branch codex/switch-port https://github.com/BIGGZ22/OpenRAC-NX.git
cd OpenRAC-NX
```

CMake downloads pinned source dependencies during the first configuration,
including the lock-strafe implementation. An internet connection is required
for that first download.

## 2. Build the Switch application

From the repository root:

```sh
cd port
cmake --preset switch-aarch64
cmake --build --preset switch-aarch64 -j8
cd ..
```

The Homebrew Menu application is:

```text
port/build-switch/openrac-rac1-pal.nro
```

Use the `.nro`, not the intermediate `.elf`. The preset uses devkitPro's
toolchain at `/opt/devkitpro/cmake/Switch.cmake` and builds only Ratchet 1 PAL.
See the [detailed Switch guide](port/platform/switch/README.md) for the explicit
CMake command and implementation details.

## 3. Optional: prepare your disc data on the PC

**You do not mount the ISO.** The tools open it directly. From the repository
root, run the following, replacing the example ISO path with your own:

```sh
python tools/extractor.py "/c/path/to/Ratchet and Clank.iso" \
  --game rac1 --proj-path build/switch-data --extract --boot-only

python editor/extract.py port "/c/path/to/Ratchet and Clank.iso" \
  build/switch-data/port/pal --jobs 2
```

The first command validates the disc and prepares the full `disc.iso`, the
boot executable `SCES_509.16`, `SYSTEM.CNF` and `buildinfo.json`. The
`--boot-only` option avoids extracting unused disc files; it still retains
and validates the full ISO.

The second command exports the level models and textures used by the native
renderer. Wait for it to finish before copying the data. The resulting folders
are `build/switch-data/iso_data/` and `build/switch-data/port/`.

The extractor rejects unrecognized disc revisions. Renaming another game's
ISO or executable will not make it compatible.

## 4. Copy PC-prepared data to the SD card

Create `/switch/openrac/` on the SD card. Copy the NRO into it, then copy both
prepared data folders into `/switch/openrac/data/`:

```text
SD:/switch/openrac/
  openrac-rac1-pal.nro
  data/
    iso_data/
      rac1/
        disc.iso
        SCES_509.16
        SYSTEM.CNF
        buildinfo.json
    port/
      pal/
        level_00/
        level_01/
        ...
        level_18/
        level_98/
        level_99/
```

Copy **all exported level folders**, including their contents. The title and
first-level manifests are checked by the launcher, but later levels are also
needed to continue playing. Keep the ISO as one file; split ISO files are not
supported. Your SD filesystem must be able to store that file.

## 5. Start the game

1. Open Homebrew Menu in full application mode and select **OpenRAC**.
2. Check that the Play page shows the disc, exported levels and save storage as ready. Press **X** to recheck.
3. Press **A** on the Play page, or **ZL**, to start once the disc and title/first-level assets
   are found.

The launch menu has **Play, Display, Audio, Gameplay, Game fixes, Saves and Setup**
tabs. Use **L/R** to switch tabs, **Up/Down** to select, and **Left/Right** or
**A** to change a setting. **B** returns to Play and **+** exits. Changes are
saved to `/switch/openrac/settings/rac1-pal.ini` and applied at game startup;
the menu reports a failure if settings cannot be written.

The launcher now uses a graphical sidebar, cards, switches and a progress bar,
with the Switch's shared system font. Touch a sidebar page or use L/R; touch
a setting's value or use Left/Right to adjust it.

**Display > Launcher graphics** switches the menu between **OpenGL** (default)
and **deko3d (experimental)**. The deko3d path uploads the custom UI into native
scanout images without an OpenGL context. It is currently a launcher backend,
**not a deko3d game renderer**: gameplay still uses OpenGL, shown separately in
the header. A full deko3d gameplay backend and gameplay-renderer switching are
not implemented yet. Both launch paths compile; console validation is required.

If native menu startup fails, the launcher tries the other presenter. An
unfinished deko3d startup is detected at the next launch and restores OpenGL.
You can also reset `switch_launcher_deko3d = 0` in
`/switch/openrac/settings/rac1-pal.ini` on the SD card.

Display options include picture shape, HUD position, resolution scale,
anti-aliasing, texture filtering, anisotropy, scenery detail, VSync, frame rate,
frame cap and frame counter. Audio offers master, music and sound volumes and
playback mode. Gameplay offers language, cutscene skipping and detailed logs.
The Game fixes tab exposes this disc's upstream fix switches. Vulkan, FSR and
desktop window/monitor controls are unavailable on Switch.

The **Saves** tab shows the memory-card folder and performs a write/close/read-back
check. Save normally through the game's own menu, wait for completion, and exit
with L + R + Minus before restarting. Back up the entire
`/switch/openrac/memcard/SCES_509.16/` folder to preserve progress. Save writes
are flushed to the SD card before reporting success. An unavailable or
unwritable card is shown as an error; existing saves are not reset.

On the Play tab, **Y** starts a 300-frame headless diagnostic.

## Gameplay controls

| Switch input | Function / PS2 equivalent |
| --- | --- |
| Left stick | Movement |
| Right stick | Camera |
| Hold ZL | Third-person lock-strafe; release for normal movement |
| B | Cross |
| A | Circle |
| X | Triangle |
| Y | Square |
| L / R | L1 / R1 |
| ZR | R2, original behavior |
| + / - | Start / Select |
| Stick clicks | L3 / R3 |
| Hold L + R + Minus | Exit at a game-frame boundary |

While lock-strafing, the left stick moves independently of Ratchet's facing
and the right stick controls camera/aim. The extension stays inactive in
cutscenes, menus, vehicles and unsupported movement states. It is built into
the NRO and does not require installing a separate mod.

## Troubleshooting and updates

- **Files missing:** check the SD layout above. A raw ISO alone is not the
  complete installation until first-launch preparation finishes. Use the Setup tab to prepare or repair it.
- **Startup or memory trouble:** use Homebrew Menu in full application mode.
- **Movie sound or strafe problems:** report the scene/planet, what happened,
  and which build you tested. These changes still need hardware feedback.
- **Logs:** the session log is
  `SD:/switch/openrac/data/openrac-rac1-pal.log`. Review logs for personal
  paths or other private information before posting them publicly.
- **Updating the application:** replace the NRO with your newly built version.
  Keep your existing game data, saves and settings unless an update explicitly
  requires a new level export. Saves are under `/switch/openrac/memcard/`.

## Credits and license

This fork builds on the work of the [OpenRAC contributors](https://github.com/OpenRAC/OpenRAC)
and the decompilation projects listed in [CREDITS.md](CREDITS.md). The
lock-strafe extension is adapted from
[OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods), with Switch-specific
input, memory access and smoothing changes. The Switch toolchain and platform
libraries come from devkitPro/libnx, SDL, Mesa and deko3d. The graphical
launcher loads the Switch's shared system font on device; no font file is bundled.

Existing licenses and notices are retained. See [LICENSE.md](LICENSE.md),
[COPYING](COPYING) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Source
changes to GPL-covered components remain covered by their existing licenses.

OpenRAC-NX is an unofficial fan project, not affiliated with or endorsed by
Sony Interactive Entertainment, Insomniac Games, Nintendo or the upstream
OpenRAC maintainers. Ratchet & Clank and other trademarks belong to their
respective owners. This repository does not distribute the game or its assets.

<details>
<summary>Upstream decompilation progress</summary>

These figures describe the upstream decompilation projects, not Switch compatibility.

<!-- badges:begin -->
<!-- Generated by tools/openrac.py tables from games/*/game.json; do not edit. -->
<table>
  <tr><th align="left" width="260">Game</th><th align="left" width="270">Project</th><th align="right" width="190">Code decompiled</th></tr>
  <tr><td>Ratchet &amp; Clank (PAL)</td><td><a href="https://github.com/OpenRAC/rac1-decomp">rac1-decomp</a></td><td align="right"><a href="https://bytematch.dev/OpenRAC/rac1-decomp/SCES_509.16"><img src="https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&amp;label=&amp;measure=matched_code_percent&amp;version=SCES_509.16&amp;color=ffb443&amp;style=flat-square" alt="Ratchet & Clank (PAL): code decompiled"></a></td></tr>
  <tr><td>Ratchet &amp; Clank (NTSC)</td><td><a href="https://github.com/lombyte-project/Lombyte">Lombyte Project</a></td><td align="right"><a href="https://bytematch.dev/lombyte-project/Lombyte/SCUS_971.99"><img src="https://bytematch.dev/lombyte-project/Lombyte.svg?mode=shield&amp;label=&amp;measure=matched_code_percent&amp;version=SCUS_971.99&amp;color=ffb443&amp;style=flat-square" alt="Ratchet & Clank (NTSC): code decompiled"></a></td></tr>
  <tr><td>Going Commando</td><td><a href="https://github.com/OpenRAC/rac2-gc-decomp">rac2-decomp</a></td><td align="right"><a href="https://bytematch.dev/OpenRAC/rac2-decomp/SCUS_972.68"><img src="https://bytematch.dev/OpenRAC/rac2-decomp.svg?mode=shield&amp;label=&amp;measure=matched_code_percent&amp;version=SCUS_972.68&amp;color=ffb443&amp;style=flat-square" alt="Going Commando: code decompiled"></a></td></tr>
  <tr><td>Up Your Arsenal</td><td><a href="https://github.com/OpenRAC/rac3-uya-decomp">rac3-uya-decomp</a></td><td align="right"><a href="https://bytematch.dev/OpenRAC/rac3-decomp/SCUS_973.53"><img src="https://bytematch.dev/OpenRAC/rac3-decomp.svg?mode=shield&amp;label=&amp;measure=matched_code_percent&amp;version=SCUS_973.53&amp;color=ffb443&amp;style=flat-square" alt="Up Your Arsenal: code decompiled"></a></td></tr>
  <tr><td>Deadlocked</td><td><a href="https://github.com/OpenRAC/rac-deadlocked-decomp">rac-deadlocked-decomp</a></td><td align="right"><a href="https://bytematch.dev/OpenRAC/rac4-decomp/SCUS_974.65"><img src="https://bytematch.dev/OpenRAC/rac4-decomp.svg?mode=shield&amp;label=&amp;measure=matched_code_percent&amp;version=SCUS_974.65&amp;color=ffb443&amp;style=flat-square" alt="Deadlocked: code decompiled"></a></td></tr>
  <tr><td>Size Matters</td><td><a href="https://github.com/OpenRAC/rac5-sm-decomp">rac5-sm-decomp</a></td><td align="right"><a href="https://bytematch.dev/OpenRAC/rac5-sm-decomp/SCUS_976.15"><img src="https://bytematch.dev/OpenRAC/rac5-sm-decomp.svg?mode=shield&amp;label=&amp;measure=matched_code_percent&amp;version=SCUS_976.15&amp;color=ffb443&amp;style=flat-square" alt="Size Matters: code decompiled"></a></td></tr>
</table>
<!-- badges:end -->

</details>
