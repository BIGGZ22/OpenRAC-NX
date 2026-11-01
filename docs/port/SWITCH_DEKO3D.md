# Deko3D experiment — source stage

This is a separate native graphics prototype, not a selectable game renderer.
The user authorized test builds on 2026-10-28. The native probe NRO now builds
with all six UAM shaders embedded in RomFS. There is still no Deko3D game NRO.
Console execution and visual validation remain pending.

## Implemented source

`port/renderer/deko3d/presenter.{h,cpp}` uses the installed devkitPro Deko3D C API:

- Native NWindow, graphics device, graphics queue and two-image swapchain.
- Retained command and upload memory per framebuffer slot.
- CPU-side fence waits before rewriting a slot still in use by the GPU.
- RGBA8 uploads, with padded row pitches, and native framebuffer clears.
- GPU image-to-buffer readback checks for color/stride/order mistakes.
- Native vertex/fragment shader loading from UAM-produced DKSH files.
- A red triangle drawn by the GPU, with a center-pixel readback check.
- Retained vertex/index buffers, per-slot transform uniforms and depth images.
- Indexed colored meshes and a near-red/far-blue depth-rejection readback test.
- Cleanup after partial initialization failures and driver diagnostics.

The presenter has no SDL, EGL, OpenGL, Vulkan, Mesa or game-code dependency. It
owns the native display while open. It must not coexist with another backend
owning the same NWindow. Readbacks are probe-only; the normal upload path does not
read the framebuffer back or wait for the entire queue each frame. Conservative
cache barriers remain during bring-up; no performance result is established.

`platform/switch/deko3d_probe.cpp` is an independent future hardware test. It
loads no ISO, settings, saves, artwork key or game. It will display red/green/blue/
white quadrants, a red triangle on blue, then a rotating indexed cube and a small native scene. Its logs are separate:

- `/switch/openrac/deko3d-probe.log`
- `/switch/openrac/deko3d-driver.log`

Pixel checks cannot establish that the physical display showed the right image.
The user must still confirm orientation, colors and presentation on hardware.

## Future build, only when requested

Use a third build directory, separate from `build-switch` and `build-switch-nvk`:

```sh
cmake -S port -B port/build-switch-deko3d \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Switch.cmake" -DSWITCH=ON \
  -DOPENRAC_PORT_WINDOW=OFF -DOPENRAC_PORT_GAMES=OFF \
  -DOPENRAC_SWITCH_DEKO3D_PROBE=ON
cmake --build port/build-switch-deko3d --target openrac-switch-deko3d-probe_nro -j8
```

The optional target links the installed debug Deko3D library. Build rules compile
the probe's GLSL shaders with the host UAM tool and package their DKSH files in
RomFS. UAM has not been invoked. Probe output is named
`openrac-switch-deko3d-probe.nro`, never the playable game's normal filename.

## Remaining game-renderer work

The primitive/mesh tests are not the game's renderer. A full game backend still
needs OpenRAC's scene formats and draw batches connected to the retained buffers,
textures/palettes, samplers, shader variants, the game's depth/stencil/blend states, render
targets/MSAA, effects/particles, UI and final composition. Shader compilation is
a particular boundary: OpenRAC currently assembles GLSL at runtime; Deko3D accepts
native shader code. A finite shader-variant scheme or a compatible runtime shader
compiler must be chosen and tested before claiming game compatibility.

Keep that work behind a separate backend boundary. Do not create stub GL calls
that report success while silently skipping game draws. Do not expose Deko3D as
a working renderer option until the required rendering paths exist.

## Checks so far

The presenter and probe pass the Switch compiler's `-fsyntax-only` checks,
including SDK argument types and the mesh/depth additions. No object files,
native shaders, linked executables or NROs were generated. This is not a linked
build or proof of synchronization/graphics correctness. Whitespace checks and
the Vulkan cache-isolation script also pass.

## Other ports reviewed

See [the reference review](SWITCH_RENDERER_REFERENCES.md) for Lost Odyssey's and
Bejeweled's shared Plume backend, shader compilation/cache approach and CPU
profiling workflow. Their image-transfer code helped identify and fix a real
unit error here: `DkCopyBuf.rowLength` and `.imageHeight` are byte strides, not
pixel/row counts. This was checked against Deko3D's `source/dk_image.cpp`.

References: [devkitPro Deko3D](https://github.com/devkitPro/deko3d) and the installed
Switch graphics examples. The library retains its upstream zlib license. OpenRAC's
new adapter/probe code uses the project's GPL-3.0-or-later license.

## Game integration: converted texture bridge

The first game-data boundary now accepts `renderer::Rgba8Image` directly through
`Presenter::upload_texture`. This is the same CPU image produced by OpenRAC's
PS2 direct-color/palette conversion code, rather than a second PS2 decoder.
Use `AlphaScale::Full` for this initial ordinary modulation shader; the GS direct
renderer requires its own 0x80-based modulation, alpha test/fail modes and blend
implementation before it can share this draw path faithfully.

The bridge retains one native RGBA8 image, an image descriptor and a sampler;
it supports nearest/linear sampling and repeat/clamp-to-edge. Source rows are
padded to the SDK's byte-pitch alignment. Replacement waits for frame consumers,
performs a synchronous loading transfer and invalidates recorded descriptor
references before retiring the previous image. There is no per-frame texture
allocation in the mesh draw path. This loading implementation intentionally does
not yet offer a texture pool, mip chains, asynchronous streaming or GS region
clamping. It does not route the live game to Deko3D.

`MeshVertex` now includes UVs. The mesh program uses a shared 80-byte std140
transform/material block, binds it to both shader stages and optionally samples
the native texture. The vertex/color/depth test remains available without a
texture. The probe now sends a 3x2 PS2 PSMCT32 image through the real game
converter (including 0x80 alpha expansion), uploads its padded rows, draws a
textured quad and checks its red center across repeated use of both frame slots.
These checks execute only when a future probe build runs on hardware.

Source-only Switch checks cover the presenter, game texture converter and probe.
The GLSL programs have now compiled with UAM and linked into the standalone
probe; they have not been run on Switch yet.
Existing OpenGL and Vulkan NROs remain unchanged.

Next integration boundaries: multi-texture ownership and material batches;
world vertex/index adapters and GPU joint palettes; game-correct reversed depth,
fog and GS alpha/blend rules; direct/UI draw submission; native frame ownership
in the platform window. The current probe's conventional depth and simple color
modulation are not substitutes for those game rules.

## Native game-frame integration in progress

`scene_renderer.h` now defines a native multi-object frame API implemented beside
Presenter's device internals. `level_scene.{h,cpp}` adapts the real
`scene::LevelData`, `scene::Vertex`, material and captured instance layouts:

- One retained level vertex/index allocation, reused for every instance.
- Native texture ownership by monotonic handles; image/sampler pairs deduplicate
  across materials and preserve independent S/T wrapping.
- Frame-owned constants, GPU joint palettes (up to 256 joints), tie/shrub light
  colours, directional light sets and camera/fog data.
- Ordered opaque, translucent cutout and metal draws; reversed GEQUAL depth.
- A bounded per-slot dynamic vertex/index ring for changing particle/HUD meshes.
- Screen-space geometry with independent scissor/depth/blend state.
- Frame draw/triangle/uniform/dynamic-byte counters and validation before submit.

CPU writes happen only after acquiring and waiting for the frame slot. Scene
command owners are recreated after that fence, avoiding the reported clear-path
leak. Two 16 MiB uniform rings and two 2 MiB command allocations are retained;
each 8 MiB dynamic ring is allocated only if used. Uniform/palette/ring overflow
fails visibly instead of drawing incomplete frames. Conservative full cache
barriers remain; this is not yet a performance-tuned implementation.

The hardware test now includes a synthetic level using the exact game data
structures, two materials and overlapping objects. One mesh starts outside the
view and only its nonidentity joint pose brings it into view; a red center
readback checks the pose and reversed-depth result. Repeated dynamic screen-space
triangles test the frame ring separately. These tests are now packaged in the native probe after UAM compilation and
linking. Their GPU readbacks have not yet been executed on a Switch.

The live game has NOT been routed to this API. Remaining prerequisites for a
playable native build include the GS direct/HUD shader and alpha-fail/blend rules,
off-screen menu targets, pause/vendor snapshots and feedback copies, particle
texture paging, sky-shell/culling/pass details, launcher drawing, and native
window/frontend lifecycle integration. Stock Deko3D 0.5 separate-alpha behavior
also needs validation; the current simple scene blends deliberately use matching
destination factors. It cannot yet express every game blend faithfully.

The full native game renderer remains unfinished, and this document does not
claim otherwise. No success-returning GL replacement functions or OpenGL fallback
have been installed as a substitute. OpenGL remains the working game path.

## Test build produced 2026-10-28

`port/build-switch-deko3d/openrac-switch-deko3d-probe.nro` builds successfully.
The package's NRO/ASET/NACP structure, 0.2.0-alpha metadata, embedded RomFS and
six DKSH signatures were checked. An explicit `dkQueueFlush` publishes work
before optional CPU readback fence waits. Packaging creates its RomFS directory
at configuration time as required by devkitPro's `nx_create_nro` helper.

See `build/renderer-test-pack-2026-10-28/TESTING.txt` for the shared test package.
Older source-only notes above describe earlier checks, not console results.
