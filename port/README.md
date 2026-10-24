# The native port

OpenRAC's native PC port: the decompiled games built for a PC and drawn by
a renderer of OpenRAC's own, the way [OpenGOAL](https://github.com/open-goal/jak-project)
ported Jak and Daxter. **No emulation**: nothing here runs the retail
program or models the console's processors, GS, vector units or DMA
controller. The design, the studies behind it and the order of work are in
[docs/port](../docs/port/README.md).

```sh
cd port
cmake --preset release            # fetches SDL3 when the system has none
cmake --build --preset release
ctest --preset release
python3 -m unittest discover -s tools/hostgen    # the translator's tests
```

You need CMake 3.24+, Ninja, Clang (it builds the port, and hostgen uses it
to read the decompilation) and Python 3. `--preset headless` builds without
a window (no SDL3): the runtime, the games and their tests. On Linux the
build links with mold or lld and compiles through ccache when they are
installed (`-DOPENRAC_PORT_FAST_BUILD=OFF` turns that off).

To build a game's program without any of that by hand, run
`python3 tools/build_port.py [GAME] [--source DIR]` from the top of OpenRAC:
the launcher's "Build the game" runs it. It configures `build/release` the
first time and then builds only what changed. On Windows it sets up Visual
Studio's x64 environment itself (the MSVC libraries and the Windows SDK) and
finds Clang and Ninja. `--source` builds from your own copy of the
decompilation and is remembered by the build folder.

Enhanced lighting (probe GI, modern shadows, GTAO) is experimental. Every
build compiles it in (`OPENRAC_ENHANCED_RENDERING`, ON by default; OFF leaves it
out), and it stays off until the `enhanced_rendering` setting or
`OPENRAC_ENHANCED=1` turns it on at start; off, the game draws exactly as a
build without it. Players turn it on in the launcher: Mods, **Enhanced
graphics**, which plays with Vulkan and bakes the probes on their computer from
their own extracted levels, one level per thread, into
`<data>/probes/<version>` (`--probes`). It needs Vulkan and a bake that matches
the level; OpenGL, and a level without one, keep the original lighting. Core
Vulkan draw/culling optimizations work either way. Only the offline baker needs
Embree: `python3 tools/build_port.py --probe-baker` builds a pinned Embree
statically into `openrac-probe-bake` (or `-DOPENRAC_PROBE_BAKER=ON` with an
installed Embree 4); the game never links it.

Dedicated enhanced-rendering sources, shaders, tools and tests live in
[`enhanced/`](enhanced/README.md). Shared renderer integration stays with its
systems, and core Vulkan performance improvements remain in the core renderer.

## What is here

| Directory | |
|---|---|
| [runtime/](runtime) | game memory, laid out as the console's (a 4 GB reservation whose offsets are its addresses), the contract translated code is written against ([guest.h](runtime/include/openrac/guest.h)), the executable's data loader, the crash handler |
| [tools/hostgen/](tools/hostgen/README.md) | writes a decompilation's C again for game memory: pointers as 32-bit game addresses, calls matched to their definitions as the EE passed arguments, calls through code addresses. It reports the frontier: what the program reaches that has no C |
| [game/](game/README.md) | the games, one directory per version (`rac1-pal`, `rac1-ntsc`, `rac2-ntsc`, `rac3-ntsc`, `rac4-ntsc`): each its hostgen configuration and the table of the libraries it calls. [game/common/](game/common) is what they share: the library replacements (disc, memory card, graphics hand-off, kernel, pads, sound) and the program, `openrac-<id>` |
| [platform/](platform) | the window, input as the console's pad, audio output, timing (SDL3) |
| [renderer/](renderer) | the renderer: per-subsystem renderers in OpenGOAL's manner, the direct renderer for the 2D path, texture conversion (OpenGL 4.1) |
| [renderer/ui/](renderer/ui/README.md) | what the game programs draw over the picture with the renderer's GL: Dear ImGui's draw lists on the Vulkan renderer's window, the on-screen pad, the frame-rate counter, the debug menu's picture |
| [renderer/vulkan/](renderer/vulkan/README.md) | the renderer's OpenGL calls carried out with Vulkan 1.3, chosen with `OPENRAC_GL_API=vulkan` |
| [scene/](scene/README.md) | a level and its live mobys, ready to draw: the editor's level export loaded, the mobys' poses and shadows read from game memory, the level drawn layer by layer through the renderer; the viewer and the game programs draw with it |
| [viewer/](viewer/README.md) | `openrac-viewer`: a level, extracted from the player's disc by the editor, drawn natively with a free camera |
| [common/](common/README.md) | what every module may use: the log, the crash report, the statistics, the system's description, a JSON reader, the PS2's float arithmetic, and small helpers (`joining_thread.h`, a thread its owner joins) |
| [tests/](tests) | the runtime's tests |
| [docs/](docs) | the port's [coding conventions](docs/CODING_CONVENTIONS.md) |
| [tools/conventions/](tools/conventions/README.md) | the conventions' countable rules, checked in CI |
| [tools/layering/](tools/layering/README.md) | the modules' layering: no include of a higher layer, today's exceptions listed |

## How a game is built

```
games/rac1 (the decompilation, read only)
   │  hostgen: Clang reads each file as the console's C,
   │  hostgen writes it again for game memory
   ▼
build/.../games/rac1-pal/gen/*.c  +  game/common/lib/*.c (library replacements, shared)
                                 +  game/rac1-pal/host/*.c (the game's own, if any;
                                    rac1's versions share game/rac1/host)
   │  compiled as C, linked with runtime/
   ▼
openrac-rac1-pal --data <install>/active/rac1/data
   loads the player's executable's data to its addresses, registers every
   function by code address, runs the game's main()
```

On macOS each windowed program is also an application bundle named by the
game's title and version, so that the Dock, Cmd-Tab, Activity Monitor and
crash reports show "Ratchet & Clank (PAL)" rather than the file name:
`build/release/apps/openrac-rac1-pal/Ratchet & Clank (PAL).app` (the title
and the version's video standard from `games/<game>/game.json`, which the
window's title shows too; see
[cmake/MacApp.cmake](cmake/MacApp.cmake)). `build/release/openrac-rac1-pal`
is a link to its program, for a shell; macOS names a program started through
the link after the link. The launcher starts the one in the bundle. A game's
bundle declares itself a game to macOS, so Game Mode turns on while it is in
full screen (F11, or Borderless or Fullscreen in the game's video options).

The program stops at the first function that has no C yet and names it.
For OpenRAC's decompilation that is the boot stage, still assembly; a copy
of the decompilation that is further along builds the same way
(`-DOPENRAC_RAC1_PAL_SOURCE=<dir>`), and hostgen's report says what it reaches
next.

Every version in [game/](game/README.md) is built the same way when its
decompilation is present. The other four build today but do not run yet:
their `main` is not identified and their library tables are partly or not
filled; [game/README.md](game/README.md) says what each needs.

## The player's settings

What the port adds to a game's own options is kept in a small text file,
`<data home>/openrac/settings/<id>.ini` (`--settings FILE` names another):
the window (a window, borderless or fullscreen; the monitor; the
resolution), the picture (its shape up to 32:9, where the HUD goes in a wide
one, the render scale), anti-aliasing and the textures' filter, vsync and the
frame rate each on its own, a frame-rate counter, and the volume.
[docs/port/SETTINGS.md](../docs/port/SETTINGS.md) lists every key and value,
and says how the frames are paced against the game's tick, what the graphics
card is checked for, where a frame's time goes, and how the settings compare
with OpenGOAL's and ReRAC's.

In Ratchet & Clank (PAL) they are the rows of the title menu's Options,
Video page, drawn by the game's widgets
([options_menu.cpp](game/common/options_menu.cpp)): Cross or Right moves a
row to its next value, Left to the one before, and the change is applied and
saved at once. The game's own row there, the television's 50 or 60 Hz, is
gone: the game ticks at its own rate (50 a second for PAL) and the window
shows it at the display's.

## Mods

Changes of play are mods, not settings of the port: shared libraries the
program loads at start through a versioned C interface
([sdk/include/openrac/openrac_mod.h](../sdk/include/openrac/openrac_mod.h)),
which take the game's hook points (`hook` rows of a version's
`libraries.tsv`, written by hostgen in `hooks.c`) and run each tick and each
pad report. The launcher installs them in `<data>/plugins/` and writes the
list the program reads, `<data>/plugins/plugins.json` (`--plugins FILE`
names another); the program writes what it loaded, and why it left any out,
to `status.json` beside it, and a `mods` line in the log's header.
`--no-mods` (the launcher's Purist mode) loads none. Without a mod the hook
points run the game's own C. The loader is
[game/common/mods.cpp](game/common/mods.cpp); the design, the manifest and
the SDK are in [docs/port/MODS.md](../docs/port/MODS.md). Ratchet & Clank's
mods, the lock-strafe first, are in
[OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods).

Script mods are `.luau` files that the program runs in Luau (fetched by
[cmake/Luau.cmake](cmake/Luau.cmake)), one sandboxed state each, with a time
budget per call ([game/common/mod_scripts.cpp](game/common/mod_scripts.cpp)):
no compiler for the modder, and every system, Android included. They reach
the same hook points through a game's binding,
[game/rac1/script_binding.c](game/rac1/script_binding.c), which
`python3 ../sdk/tools/gen_binding.py` writes from
[sdk/bindings/rac1.json](../sdk/bindings/rac1.json) (a new `hook` row needs
its line there; ctest's `mod_binding` says when they disagree).

## Direct level probes (RAC1 PAL)

For development, `OPENRAC_DIRECT=1` with `--level 0` starts a fresh game
in Veldin once the title world has initialized. It calls NewGameInit and
the normal level loader without menu input. Add `--skip-movies` to return
the movie player's skipped result immediately, including for intros.
Every skipped movie is logged. This flag is off by default.

From a configured PowerShell build environment, for example:

```powershell
$env:OPENRAC_DIRECT = '1'
$env:OPENRAC_PRESS = '100:4000:5' # acknowledge the initial no-card warning
.\build\release\openrac-rac1-pal.exe --data <extracted-data> --window --no-card --level 0 --skip-movies --stop-on-missing
Remove-Item Env:OPENRAC_DIRECT
Remove-Item Env:OPENRAC_PRESS
```

Use an isolated card directory if testing saves. Direct probes exercise
fresh-game level loading, not saved progression or menu/movie playback.
Keep `--stop-on-missing` enabled and also run the normal New Game path
without these shortcuts when validating startup behavior.

## Debugging

F3 shows a debug overlay over the game: frame times, the renderer, memory,
the mobys with an inspector, the game's state and the shaders, with pause,
single step and slow motion. `--debug-dump FILE` writes each frame's
measurements as JSON lines for automated checks.
[docs/port/DEBUGGING.md](../docs/port/DEBUGGING.md) describes them and
lists every `OPENRAC_*` environment variable.

To diagnose controller input, set `OPENRAC_TRACE_INPUT=1`: it logs the pad
report passed to the game every 25 frontend frames in which a port is read,
including scripted input. With `OPENRAC_DEBUG=1`, RAC1 PAL also logs the
processed pad state and stick axes every 100 frontend frames. Compare these
with the player position to distinguish device input from game-state bugs.

## Rules

OpenRAC's rules hold here ([AGENTS.md](../AGENTS.md),
[SOURCING.md](../docs/policy/SOURCING.md)): nothing from a disc is ever
committed (no data, code bytes, microcode or media), and the library
replacements are written from what the game's own code shows and what is
publicly known of each API, never from Sony's code or documentation.
Generated C and build output stay in build trees (`port/build/` is ignored).

The port's code follows [docs/CODING_CONVENTIONS.md](docs/CODING_CONVENTIONS.md)
(decided): layout ([.clang-format](.clang-format)), comments and documentation,
naming, ownership, errors, the C boundary and the module layering.
[tools/conventions](tools/conventions/README.md) checks what a program can
count, in CI and before you commit, [tools/layering](tools/layering/README.md)
that no module includes a higher one, and [.clang-tidy](.clang-tidy) the
naming and ownership of the C++ you touch.

The port is built on Linux, macOS and Windows (there with Clang against
MSVC's libraries); CI builds all of it on Linux and rac1/pal's program on
Windows. Host C and C++ keep to the standard library and to what the runtime
offers: a POSIX-only header or function (`execinfo.h`, `unistd.h`, `fseeko`,
...) goes behind `_WIN32` with its Windows form beside it, as in
[runtime/](runtime), and a debugging aid that wants the host's stack calls
`openrac_host_caller` ([guest.h](runtime/include/openrac/guest.h)) instead of
`backtrace`.

GPL-3.0-or-later, as the rest of OpenRAC's top level.
