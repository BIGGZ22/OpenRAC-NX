# Claude handoff: persistent Deko3D geometry corruption

## User's current instructions

Continue fixing actual models/scenery in OpenRAC-NX. **DO NOT BUILD** until the
user explicitly authorizes another build. This hold applies after the last
successful candidate below. Prioritize playable, correct graphics; do not work
on mods or unrelated menu features. Preserve all local work. Nothing in this
session was committed or pushed. Do not reset, checkout over, stash away or
replace the working tree. Explain findings plainly and distinguish confirmed
facts from hypotheses. User is frustrated with repeated diagnostic builds that
have not visibly improved the game.

Repository: C:/msys64/home/Biggz/OpenRAC, MSYS /home/Biggz/OpenRAC.
Read the original docs/port/HANDOFF_2026-10-29.md in full, then
docs/port/SWITCH_DEKO3D_SHADERS.md and port/renderer/dkgl/dkgl.h. This document
supplements them; it does not supersede historical hardware measurements.

## Current renderer and exact tested candidate

OpenGL is the stable reference. NVK Vulkan runs but has high per-draw overhead.
The actual native game renderer is **port/renderer/dkgl/**, not the earlier
port/renderer/deko3d/Presenter experiment. dkgl implements the game's GL calls
directly on Deko3D and loads nine precompiled shader programs from RomFS.
Installed SDK is **Deko3D 0.5**, so inspect v0.5.0, not only master.

Last successful build, already tested by user:

- build/deko3d-test/openrac-rac1-pal-deko3d.nro
- NRO bytes 29088209
- NRO SHA256 f82786111131bf1ae10cdc277dd09dea80064045ebf77a6d60da6216dc5c7e3c
- matching build/deko3d-test/last-build.elf
- ELF SHA256 3c141a3d059f8357b480b52456be989d4ae5c6a6408a21255ae2d4a7f1d23244
- build ID f1608851276abe6455abbbcde98dbd3c3a30679a
- build/deko3d-test/last-build.json records identity.

Latest supplied evidence, October 29, after that candidate:

- C:/Users/Biggz/AppData/Local/Temp/openrac-rac1-pal (22).log
- C:/Users/Biggz/AppData/Local/Temp/dkgl-driver (9).log
- C:/Users/Biggz/AppData/Local/Temp/deko3d-startup (11).log
- D:/Downloads/2026102903093100-03DB12780BD84000.mp4
- D:/Downloads/2026102903101900-03DB12780BD84000.mp4

Copied logs, tested NRO/ELF/manifest and extracted frames are archived in
build/deko3d-test/diagnostics/2026-10-29-upstream-graphics-result/.
The MP4 files remain at their original paths. At 15 seconds the first clip shows
the memory-card menu. The second shows Novalis/vendor view with detached pieces,
black rectangular/triangular surfaces and a broken preview. Inspect additional
times; one extracted frame is not a whole video review. Older clips show the
malformed landing ship, menu Ratchet and white scenery shards. Pyrocitor texture
now appears, but its model remains malformed. User says the latest looks the same.

## What is confirmed and what is NOT

Latest session:

- All **17 real-GPU pixel tests PASS**; GL error zero.
- No `dkgl geometry:` range-rejection entries, no sampled `finite false`.
- No error/warning lines and no camera-guard rejection/recovery messages.
- Native shader fingerprints match the newly rebuilt pack. Mesh vertex is
  7936 bytes, FNV 4252434357788652; mesh fragment 2304 bytes, FNV b8ca74a9da19916b.
- Driver (9) is driver (8) as an exact prefix plus one queue-creation line.
  Old GPU method errors in that cumulative file are **historical**, not a new
  failure. Always compare appended content before diagnosing another GPU crash.
- These results do NOT establish correct arbitrary model vertices, whole-buffer
  contents, complicated clipping, projection, material passes or preview targets.
  They also do NOT prove the user copied the wrong build: shader identity now
  provides positive evidence that they ran the new shader code.

Do not repeat the claim that texture/palette uploads or culling are simply broken
without a counterexample from real data. Do not describe the shader rebuild or
upstream integration as a confirmed graphics fix: hardware says defects persist.

## Confirmed crash cause and completed fix

Deko3D v0.5.0 dkCmdBufBindVtxBuffers reserves **5*n words but writes 6*n**.
Verified against both SDK source and installed libdeko3dd.a disassembly. Near a
command-memory boundary it corrupts GPU commands/CPU heap. Original method error
was [b197:739] VertexArray[14].Start high word, followed by a cleanup heap crash.

port/renderer/dkgl/vertex_bind_compat.h captures the SDK packet in a dedicated
scratch command buffer with ample space and a canary, checks exact length, then
replays to the live buffer with correct reservation. Context::vertex_capture
is created/destroyed in dkgl_core.cpp. Keep this fix. Hardware runs since it have
passed the former crash point. Do NOT capture on the live command buffer:
BeginCaptureCmds clears it. Native vertex-binding regression passes.

## Other changes already present

1. draw_state_compat.h fixes SDK 0.5's separate-alpha blend packet: SDK mistakenly
   writes dstColorBlendFactor into FuncAlphaDst. Captures/validates all fields and
   repairs only the affected field, caches by semantic state. Winding mapping
   compensates the pack's vertex Y flip with UpperLeft device origin. Four
   hardware culling tests now verify both GL_CCW/GL_CW and FRONT/BACK behavior.
2. Newly created sampler descriptors are invalidated in the SAME draw, after
   sampler creation, rather than on the following draw.
3. transfer_validation.h checks staging size/pitch, mip rectangles and overflow;
   unsupported texture conversions are reported. RGBA32F is copied correctly.
4. geometry_validation.h has overflow-safe indexed byte-range and vertex-fetch
   span helpers. All indexed draws validate index storage. glBufferSubData checks
   bounds without signed offset+size overflow. Sampled full index scans validate
   each reflected enabled input against vertex/instance buffer storage.
5. diagnostics.h supports no-effects, no-shadows and bind-pose, via
   OPENRAC_DK_DIAGNOSTIC. Default none. Bind-pose overrides only the GPU uniform
   snapshot. Its previous clip was not a decisive matched-pose comparison.
6. scene/dkgl_selftest.cpp runs once using the ACTUAL shipped mesh shader and
   native Vertex/120-byte instance layout. It covers nonidentity instance matrix,
   index byte offset, RGBA32F vertex sampling, unsigned-byte joint indices, palette
   subimage updates, crossing a 1024-wide palette row, queued texture updates,
   queued instance/uniform snapshots, reverse-Z occlusion, two simultaneous
   instances, four culling conventions and four weighted rotated joints.
   Tests are tiny triangles, mostly identity projection; no real mesh replay.
7. Bounded actual-geometry logging in audit_geometry (dkgl_gl.cpp), every sixty
   renderer frames, at most sixteen sampled draws per sampled frame and 4096
   snapshots/process. Reserves samples per program AND rigid/skinned mode, then
   rotates through extra draws. Logs source indices, buffers/versions, offsets,
   strides, source position/matrix/weight/palette values, finiteness, projection,
   skinning, sampler units/dimensions/formats, depth/cull/blend/target. Sampled,
   NOT exhaustive. Does not mirror GPU palette pixels or evaluate CPU skinning.
   This logging affects performance; don't benchmark it as an optimized build.

## Recent upstream integration, deliberately scoped

Fetched OpenRAC/OpenRAC upstream/develop to 9161f193. Backported relevant changes:

- fb740d63 (#110): hostgen lower.py zero-initializes unset scalar/aggregate locals
  (excluding va_list). Full PAL generated game C was regenerated/recompiled.
  Upstream diagnoses this UB as the source of missing Drone Device drones and
  certain Novalis sparks piled at a collision position. Larger moby-pool toggle
  is also present, default on, with native regression passing.
- 24d97134 (#114): usable_camera/usable_view guards, last-good-camera fallback,
  bounded logs, guarded world/packet decisions. Switch FOV scaling retained.
  Latest tested run triggers no rejection; don't assert it caused these defects.
- 9954a3bd (#108): use game's level background colour and preserve it in retained
  world camera; existing sky sprite texture override path. No new mod UI work.

Exit menus, Android mod APIs, desktop recording expansions and unrelated co-op
glove #120 were intentionally excluded after user emphasized graphics. We did
not wholesale drop the old PAL camera-return or item-pose host backfills: newer
upstream assumes decompilation imports absent from this branch. Avoid importing
all latest host replacements/libraries blindly. #106 is a newer decomp sync worth
reviewing if you need matched-C changes; preserve the custom Switch backfills.

Integration snapshots in build/deko3d-debug-reference/upstream-integration/ record
per-file current/base/upstream and trial merges. Its conflicts.json is a record
of the FIRST partial merge, not an assertion that worktree conflicts remain.
Do not rerun integrate_upstream.py or resolve_upstream.py: they are historical,
one-shot staging helpers and can overwrite later work.

## Shader tooling and what rebuilding established

Official Khronos SPIRV-Cross source and a Windows build are available:
build/deko3d-debug-reference/spirv-cross-src/
build/deko3d-debug-reference/spirv-cross-build/spirv-cross.exe
Add C:/msys64/mingw64/bin to PATH for its runtime DLLs.
Installed UAM: C:/msys64/opt/devkitpro/tools/bin/uam.exe.

port/tools/deko3d/dk_rebuild.py now rebuilds existing .prog embedded SPIR-V into
a SEPARATE output directory, with SPIRV-Cross GLSL460 --no-es and vertex-only
--flip-vert-y, then UAM. Nine programs/eighteen stages compile. Seventeen binaries
changed from the older pack. Reflection .prog bytes remain unchanged. Exact tool
hashes and intermediate/binary hashes are in renderer/dkgl/pack/provenance.json.
Current regenerated pack is installed in the working tree and was hardware
tested in session22. **It did not visibly fix the geometry.**

Also fixed actual CMake packaging dependency: file(GLOB CONFIGURE_DEPENDS) only
notices changed file lists. Games.cmake now adds the actual pack files to
CMAKE_CONFIGURE_DEPENDS, so modified DKSH bytes cause recopy into RomFS.
All eighteen staged DKSH hashes matched the source pack before last build.
Runtime load_program logs stage lengths/FNV to verify what the NRO actually uses.

Previous audit artifacts:
build/deko3d-debug-reference/shader-audit/ has GLSL, SPIR-V, SPIRV-Cross reflection
and original reflection for all nine programs. interface-audit.json checks 133
uniform/sampler fields; vertex-varying-audit.json checks 67 expanded vertex columns
and 33 varyings. Both pass. mat4 model occupies locations4..7; do not mistake the
four glslang vec4 columns for a reflection mismatch with SPIRV-Cross's mat4.
mesh.vert.tgsi is available, generated with installed UAM -t.

## Code map and useful concrete values

- dkgl_gl.cpp prepare_draw binds shaders, snapshot UBOs, sampler uniforms and
  per-attribute vertex buffers. Each attribute has its own deko buffer binding;
  GL pointer offset becomes buffer base address, hardware attr offset stays0.
- resolve(Buffer&) uploads stable buffers to 32MiB heaps, dynamic data into a
  48MiB per-frame ring. Two slots/fences, version+ring_epoch caching. Never reuse
  the earlier frame's storage before its fence.
- dkgl_core.cpp begin_list already invalidates descriptors/image/shader/L2.
  Don't propose adding the same initial invalidation as if it were missing.
  barrier increments barrier_epoch; upload uses sampled_epoch to order writes.
  Verify SDK ordering semantics before changing it. DkBarrier_Primitives (3)
  completes all prior primitives/compute; it is stronger than Fragments (2).
- CPU code writes CpuUncached|GpuCached memory. Investigate intra-list reuse,
  visibility and allocation retirement only with an actual address/lifetime trace.
- Native scene Vertex stride72; instance stride120: model4..7,tint8,palette9,
  uint8 joints10,float weights11,light_slot12,lights13,moby_light14,ambient15.
- Mesh palette is RGBA32F width1024, four texels per matrix, base in matrices.
  First skinned geometry samples in session21 used indices62596..68178: the
  synthetic tests do not cover high vertex indices or multiple mesh groups.
- Example palette texture1024x1, native format37; unrelated bindings can be RGBA8
  when skinning0. Do not treat an unused palette sampler's RGBA8 texture as proof
  of a palette failure. Audit logs show the skinning uniform alongside the binding.
- Renderer uses reversed depth GEQUAL; EQUAL only when reusing a world depth
  prepass. Both GLSL mesh/depth declare invariant gl_Position. The hardware test
  checks reverse-Z with one shader, not full mesh-depth vs colour invariance.
- source shaders: port/viewer/shaders/mesh.vert, mesh_depth.vert, mesh.frag.
- world/render call ordering: port/renderer/renderer.cpp,
  port/scene/level_renderer.cpp, game/common/frontend/scene_frame.cpp.

## Recommended next investigation: real data, not another generic triangle

These are hypotheses/work directions, NOT confirmed causes:

1. Capture ONE actually broken model draw (menu Ratchet, landing ship or vendor
   weapon) with stable model/group/packet ID, tick, program, target and material.
   Audit logs currently identify GL objects but don't tie them back to semantic
   game model IDs. Add scoped capture at LevelScene submission if needed.
2. Capture full CPU index/vertex/instance inputs for that selected draw plus its
   exact palette CPU shadow, active uniform snapshot and texture metadata.
   CPU-shadow only the needed float palette in debug mode; mirror TexImage and
   TexSubImage correctly, record version. Avoid dumping entire game assets or
   scanning every draw every frame. Include joint indices (currently not logged
   by float-only audit), all four weights, actual palette base and matrix count.
3. Compute reference positions with the SAME column-major arithmetic as
   mesh.vert: weighted joint matrix, bind vertex, force p.w=1 after skinning,
   instance model, view_projection, Vulkan z remap and pack Y flip. Check finite
   clip coordinates, near/far clipping, w signs, AABB and a small set of triangle
   vertices. Don't assume every game model is skinned: palette<0 or weights0
   deliberately takes the rigid path. Don't normalize weights unless source does.
4. Compare actual GPU geometry to CPU reference for that captured draw. Reuse an
   offscreen replay or diagnostic wireframe/solid material on the same inputs.
   A GPU diagnostic shader needs a matching source hash, reflection and rebuilt
   pack; no runtime GLSL compiler exists. Hold all game builds per user request.
   A targeted CPU-pretransformed fallback can distinguish vertex processing from
   index/state faults, but must handle per-instance offsets and normals properly;
   don't permanently force bind pose or disable culling to hide missing surfaces.
5. Test high indices >65535, real index offsets, nonzero instance pointer offsets,
   many groups, packet switching and buffer updates across frame-slot reuse.
   Tiny symmetric triangles with identity projection don't validate these.
6. Isolate actual pass state: colour/depth program position agreement if prepass
   is active; offscreen preview target/viewport/scissor/clear, clip distances,
   colour mask/stencil and target-switch synchronization. Culling conventions are
   tested but full game transitions are not. Compare against stable GL path.
7. Review native command buffer packet sizes using installed SDK source/disassembly
   for any OTHER calls with >expected writes. One such genuine SDK defect was
   already found; do not apply master-only APIs or unverified magic register fixes.

Don't conflate fixes for GS emulation, PS2 render-state reconstruction and native
GPU mesh submission. The reference ports below offer techniques, not drop-in
game-geometry translations. Source evidence plus one reproducible failing draw
will be more useful than another broad optimization pass.

## Sources already inspected, and how to search next

PRIMARY SDK/compiler sources:

- https://github.com/devkitPro/deko3d/tree/v0.5.0
- https://github.com/devkitPro/deko3d/blob/v0.5.0/source/maxwell/gpu_3d_vbo.cpp
  Reservation defect; vertex-buffer/divisor/index buffer encoding.
- https://github.com/devkitPro/deko3d/blob/v0.5.0/source/maxwell/gpu_3d_state.cpp
  AlphaDst encoding, cull/front-face and depth/stencil.
- https://github.com/devkitPro/deko3d/blob/v0.5.0/source/maxwell/gpu_3d_base.cpp
  viewport origin, zero-to-one depth, barriers, render-target setup.
- https://github.com/devkitPro/uam/blob/master/README.md
  Explicit resource bindings and separable stages. Check installed compiler
  behavior rather than assuming GLSL linking repairs stage mismatches.
- https://github.com/KhronosGroup/SPIRV-Cross
  CLI output, stage interfaces, explicit locations and coordinate conversion.

KNOWN REFERENCE PORTS:

- https://github.com/matthewperiut/night100
  https://github.com/matthewperiut/night100/blob/main/src/graphics/deko3d_backend.cpp
  Native PS2 recompilation port. Useful vertex skinning, buffer/fence lifetimes,
  state capture and the separate-alpha SDK workaround. Local source:
  build/deko3d-debug-reference/night-backend-current.cpp.
- https://github.com/centollOS/SwitchWaker
  https://github.com/centollOS/SwitchWaker/blob/main/switch/deko3d/PATCHES.md
  Documents SDK patches including separate alpha and command-memory leak issues.
  Local switchwaker-patches-current.md and build/graphics-references/sw-*.
- https://github.com/NaGaa95/NetherSX2_nx
  Previously inspected revision a78cb5836adb9024e6b0cd9b5860a526d4dda68b.
  Local build/deko3d-debug-reference/nether-*. Its GS vertex layout and LowerLeft
  device convention differ from dkgl. Useful synchronization/epoch tracking and
  bounds checks, not an excuse to copy its origin assumptions. Some copied
  reference blend code itself still relied on the SDK defect: verify everything.
- https://github.com/Ad0ky/LostOdysseyRecomp/releases/tag/v1.0.6D_NX
  Plume/deko backend copies under build/graphics-references/lost-*. Useful native
  resource binding and staging conventions, not a direct dkgl shader replacement.
- https://github.com/ChanseyIsTheBest/bejeweled3-nx/blob/main/docs/optimising-functions.md
  Profiling guidance. This is not evidence that a geometry defect is CPU load.

UPSTREAM GAME SOURCES:

- https://github.com/OpenRAC/OpenRAC/commits/develop/
- https://github.com/OpenRAC/OpenRAC/pull/110
- https://github.com/OpenRAC/OpenRAC/pull/114
- https://github.com/OpenRAC/OpenRAC/pull/108
- PR #106/decomp synchronization deserves dependency review before dropping host C.

Useful GitHub searches: "deko3d dkCmdBufBindVtxBuffers reserve", "deko3d
VertexArray Start", "deko3d instance divisor firstInstance", "deko3d buffer
CpuUncached GpuCached invalidate", "deko3d shader clip distance", "uam invariant
gl_Position", "deko3d render target texture barrier", "deko3d PATCHES AlphaDst".
Search source and issues in the named repositories first. Pin revisions and
compare to installed0.5. Validate call arguments and draw lifecycle. Don't rely
on snippets without their state setup, commit date or coordinate convention.

## Verification and tooling available without another game build

- Native regression executables in build/deko3d-debug-reference/: geometry-test,
  vertex-binding-test, draw-state-test, transfer-test, diagnostics-test,
  frontend-geometry-test and moby-pool-test. Prior runs pass. Recompile an affected
  native unit regression if source changes; do not trigger a game build.
- Upstream zero-init test passes via run_zero_init_test.py and temporary
  win-posix/sys/mman.h Windows allocation adapter, with native clang on PATH.
  Stock test skips Windows; MSYS native clang lacks sys/mman.h. Production lower.py
  and original upstream assertion are used. These helper files are local artifacts.
- Python: C:/Users/Biggz/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe
- Native g++/clang and runtime DLLs: C:/msys64/mingw64/bin (add to PATH).
- ffmpeg: C:/Users/Biggz/AppData/Local/Microsoft/WinGet/Links/ffmpeg.exe.
- Git: C:/Program Files/Git/cmd/git.exe.
- Cross addr2line/readelf: C:/msys64/opt/devkitpro/devkitA64/bin/.

When the user eventually authorizes building, claude-build-deko3d.sh or
build/deko3d-test/build-deko3d.bat builds the separate game target. The script can
exit0 even if the inner build fails! Inspect `=== exit code 0`, built target,
actual file timestamps and output/copy hashes. On failure it removes copied NRO.
Never ask user to test an unverified build. Keep ELF paired with that NRO.
SD env: /switch/openrac/env.txt; logs include /switch/openrac/data/openrac-rac1-pal.log.
No changes to stable GL or Vulkan binaries were built during this latest pass.

## Where to continue

Review the latest clips and audit samples, pick one failing draw, and trace actual
CPU-to-GPU transformed positions and state. Save supported source fixes and a
clear explanation of evidence. **No further game builds until authorized.**
