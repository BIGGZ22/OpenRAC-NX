# Experimental Switch Vulkan backend

This is a hardware bring-up path, disabled in normal Switch builds. OpenGL remains
the default when `OPENRAC_SWITCH_VULKAN=OFF`. Do not replace a known working NRO
until testing the separate probe on a console.

## Driver

Standard devkitPro portlibs do not supply Vulkan. The initial experiment uses
[jhuz0/nvk-switch](https://github.com/jhuz0/nvk-switch), release
`nvk-switch-mesa-25.0.7-r1`, a self-contained static NVK archive based on
[PalindromicBreadLoaf/nxvk](https://github.com/PalindromicBreadLoaf/nxvk).
Preserve that project's NOTICE and corresponding source when distributing it.
The archive is a local build input, not committed or automatically installed.

This integration requires a self-contained archive with internal Mesa symbols
localized. An arbitrary unlocalized `libnvk.a` is not a substitute: the launcher
and OpenGL fallback also link the portlibs' Mesa. Volk's instance-proc pointer is
renamed to avoid colliding with NVK's exported function.

## Build

Use a separate build directory and the existing devkitPro Switch toolchain:

```sh
cmake -S port -B port/build-switch-nvk \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Switch.cmake" \
  -DSWITCH=ON -DOPENRAC_PORT_WINDOW=ON \
  -DOPENRAC_PORT_GAMES=OFF -DOPENRAC_SWITCH_VULKAN=ON \
  -DOPENRAC_SWITCH_VULKAN_LIBRARY=/absolute/path/to/libvulkan.a \
  -DOPENRAC_ENHANCED_RENDERING=OFF -DOPENRAC_FSR3=OFF
cmake --build port/build-switch-nvk --target openrac-switch-vulkan-probe_nro -j8
```

Dependencies use the existing pinned Vulkan-Headers, volk, VMA and glslang versions
from `Dependencies.cmake`. Their `FETCHCONTENT_SOURCE_DIR_*` options can point to
local sources for offline builds.

## Hardware check

Launch `openrac-switch-vulkan-probe.nro` in full application mode. It does not
read the game ISO, game settings or saves. Expect a red triangle on a blue
background for about ten seconds with vsync; Plus exits early.

Read `/switch/openrac/vulkan-probe.log` and `vulkan-startup.log` afterward.
The probe explicitly rejects an OpenGL fallback as a Vulkan success. It checks
device startup, clear/readback and GLSL compilation, pipeline creation and triangle
readback. Visual presentation still needs confirmation on the console. A completed
presentation loop is not proof that the screen displayed the correct image.

The backend requires Vulkan 1.3, dynamic rendering, a graphics/present queue and
`VK_NN_vi_surface`. The native surface uses libnx's default NWindow, independent
of SDL2's missing Vulkan window backend. Shader and pipeline caches live under
`/switch/openrac/cache/vulkan`. Optional present-ID/wait/latest-ready extensions
are disabled during bring-up.

## Game integration

The Switch Window implementation selects Vulkan in experimental builds and routes
graphics entry points and swaps to the existing Vulkan renderer. New experimental
builds stop on Vulkan initialization failure by default. For explicit fallback
testing only, `OPENRAC_SWITCH_VULKAN_FALLBACK=ON` allows OpenGL and records that
fact in `vulkan-startup.log`; a fallback is never a passing Vulkan probe.
Normal OpenGL builds retain their existing path.
Build games only after the probe passes, using the existing game-source/hostgen
options with `OPENRAC_PORT_GAMES=ON` in this separate directory.

The future game artifact is named `openrac-rac1-pal-vulkan.nro`, with the package
title `OpenRAC Vulkan Test - rac1-pal` and a `VULKAN TEST` launcher label. The build
target remains `openrac-rac1-pal_nro`. It can sit beside the normal OpenGL NRO.
Game data, saves and launcher preferences keep their existing locations.

Native display sizes are read from the NWindow for initial render targets,
swapchain sizing, presentation and resizing. SDL-only mouse/safe-area and desktop
window-sizing code no longer assumes that native Vulkan has an SDL window.

## Upstream review: 2026-10-28

Fetched upstream: `master` remains at `6360471e` (movie language, already adapted).
The development branch `feat/coop-sweep` advanced to `fcac1d9a`. Backported
[`06379ee7`](https://github.com/OpenRAC/OpenRAC/commit/06379ee7997216c6c5484aa6ceb50ba442580949)'s
shader-aware Vulkan pipeline cache invalidation. The adaptation hashes standalone
shader text, included GLSL snippets and inline shaders, as well as backend sources.
It scans source directories only, excluding other build folders and test shaders.

The other recent changes concern multiplayer/menu/world APIs whose underlying
modules are absent in this fork. They were reviewed and not imported wholesale.

`port/tests/vulkan_pipeline_key_test.cmake` passes as a source-only CMake script:
shader, snippet, inline-shader and backend edits invalidate the key; unrelated
source, tests and other builds do not. This does not compile or configure a game.
These latest renderer changes have **not been linked or packaged**, per the
request to hold builds. Syntax-only Switch compiler checks now pass for the
Vulkan backend/native platform and affected game frontend; the corresponding
desktop window source also passes native syntax-only checking. These checks
produce no object files or executable builds. Existing test NROs are unchanged.

## Runtime failure handling (source changes, not packaged)

Native display acquisition has a bounded wait so a stalled display does not
block forever inside one acquisition call. Resize/surface-loss results retain
their retry path. Fatal acquire, queue-submit and present failures record a
sticky error, leave the normal frame-advance path, and reach the frontend's
existing render-error handling. Native runtime errors also write
`/switch/openrac/vulkan-runtime.log`.

A rejected queue submission is not marked submitted and is not followed by a
present that waits on its unsignaled semaphore. Failed frame-fence resets/waits
are reported explicitly. Status Monitor telemetry is updated only after a
presentation was accepted; the picture/cutscene path reports actual frames too.
These failure paths and physical display behavior still require console tests.

Pending console validation: game compatibility, launcher-to-game graphics handoff,
sleep/resume, handheld/docked transitions, memory use and crowded-scene CPU/GPU
timings. Frame generation is not part of this initial integration. No performance
improvement is established by compiling or by passing the triangle probe.

The [Deko3D experiment](SWITCH_DEKO3D.md) is a third, independent source-stage
prototype. Its shaders are excluded from Vulkan's pipeline key; it does not
replace or enable any part of the Vulkan game backend.

## Test builds produced 2026-10-28

The user authorized all renderer test builds. The current Vulkan game now builds
successfully as `port/build-switch-nvk/openrac-rac1-pal-vulkan.nro`, alongside a
refreshed `openrac-switch-vulkan-probe.nro`. OpenGL fallback was disabled in the
platform build. Game NACP metadata identifies `OpenRAC Vulkan Test - rac1-pal`
with version `0.2.0-alpha`. The launcher uses its existing OpenGL surface and
releases it before Vulkan gameplay initialization; it is not a Vulkan launcher.

Both outputs are included in `build/renderer-test-pack-2026-10-28.zip`, alongside
OpenGL and the standalone Deko3D graphics probe. NRO asset bounds, package names,
versions and archive hashes were checked. This establishes successful compilation
and packaging, not hardware compatibility or confirmed playable gameplay. Test
the driver probe first and collect its logs before attempting the game.
