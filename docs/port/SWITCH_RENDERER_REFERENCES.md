# Switch renderer references reviewed

Review date: 2026-10-28. No game builds requested or produced during this review.

## Lost Odyssey Recomp v1.0.6D_NX

[Release](https://github.com/Ad0ky/LostOdysseyRecomp/releases/tag/v1.0.6D_NX).
Its Deko3D implementation is a real application backend behind Plume's rendering
interface, not a replacement DLL or a renderer-selection flag alone.

- [Backend provenance](https://github.com/Ad0ky/LostOdysseyRecomp/blob/v1.0.6D_NX/thirdparty/plume-deko3d/README.md):
  adapted from the sa2-nx/banjo-nx Plume backend, with Lost Odyssey layout changes.
- [SPIR-V translator](https://github.com/Ad0ky/LostOdysseyRecomp/blob/v1.0.6D_NX/thirdparty/plume-deko3d/plume_deko3d_spirv.cpp):
  SPIRV-Cross generates GLSL, descriptor sets/bindings map to native slots,
  push constants map to a uniform buffer, and unsupported language constructs
  get explicit handling. It rejects unsupported resources instead of silently
  dropping them. Its input expects separate textures/samplers, unlike some of
  OpenRAC's combined sampler resources.
- The backend uses a patched UAM library on a controlled compiler worker and
  loads DKSH native shader code. Most shipped shaders come from a precompiled
  pack; missing variants can compile at runtime and persist in a separate cache.
- [Shader cache](https://github.com/Ad0ky/LostOdysseyRecomp/blob/v1.0.6D_NX/LostOdysseyRecomp/gpu/deko3d_shader_cache.cpp):
  an indexed, optionally compressed pack plus an append-only runtime cache, with
  recovery of complete records after an interrupted write.
- [Commands/synchronization](https://github.com/Ad0ky/LostOdysseyRecomp/blob/v1.0.6D_NX/thirdparty/plume-deko3d/plume_deko3d_commands.cpp):
  explicit descriptors, image transitions/cache work, deferred resource retirement
  and fences. A slow fence never grants permission to reuse GPU-owned memory.

The release reports performance gains in its own game, hardware and settings.
Those measurements are not an OpenRAC estimate. Its dynamic-resolution artifact
warning is a reason to validate viewport, image-copy, target-size and cache
transitions carefully rather than copying game-specific workarounds blindly.

## Bejeweled 3 NX

The linked [optimising-functions document](https://github.com/ChanseyIsTheBest/bejeweled3-nx/blob/main/docs/optimising-functions.md)
is primarily about CPU optimization of recompiled PowerPC functions, not Deko3D
initialization. Its useful workflow is console profiling, exactness checks and
comparison of equivalent scenes at the same clock. It explicitly distinguishes
PC proxy results from Switch performance. The address-specific hooks and register
localization machinery are not drop-in replacements for OpenRAC's PS2 host code.

The repository does have a Deko3D renderer:

- [Deko3D bridge and caches](https://github.com/ChanseyIsTheBest/bejeweled3-nx/blob/main/app/src/renut_engine/gpu/banjo_renut_deko3d.cpp):
  the same Plume backend lineage, a packaged GLSL catalog, a DKSH cache keyed by
  compiler/optimization flags, and a controlled compiler worker. It exposes
  tuning for depth compression, Zcull and barrier management.
- [Upload ring](https://github.com/ChanseyIsTheBest/bejeweled3-nx/blob/main/app/src/renut_engine/gpu/render/upload_ring.cpp):
  persistently mapped chunks, retained capacity, reuse bounded by frame ownership,
  and publication/flush of written ranges only when memory is CPU-cached.

## Application to OpenRAC

1. Keep Vulkan as the first route to gameplay; its complete game renderer already
   exists. Finish and test native display/presentation integration.
2. Build Deko3D against a real rendering boundary, not success-returning GL stubs.
   Retained geometry, frame-owned constants, depth and synchronization are now
   represented in the native prototype; scene textures/materials/UI still need work.
3. Reuse OpenRAC's shader assembly/reflection where possible. Evaluate a shared
   SPIR-V frontend plus SPIRV-Cross and patched UAM for native code, or a complete
   ahead-of-time catalog of OpenRAC's own base-renderer shader variants. Either
   route needs OpenRAC-specific binding and coordinate rules.
4. Shader cache identity must include compiler version/flags, native resource
   layout and shader text. Disk writes must tolerate interruption. Do not ship
   either reference project's game-specific shader packages in OpenRAC.
5. Use actual Switch profiles before optimizing enemy/bolt simulation. Preserve
   PS2 float rounding and guest-memory behavior; don't apply unsafe fast-math or
   transplant PowerPC hooks based on PC timings.

Concrete fix from the review: our four image-copy buffer descriptors now supply
byte pitches/layer strides. The units are confirmed by
[Deko3D's copy implementation](https://github.com/devkitPro/deko3d/blob/master/source/dk_image.cpp).
Syntax-only Switch compiler checks pass after the fix. Native shader compilation,
linking and console execution remain pending; existing test NROs are unchanged.

The synchronization review also informed Vulkan failure handling: rejected
submissions must not be counted as submitted, waited on, or presented through an
unsignaled semaphore. Native display failure reporting and accepted-frame overlay
accounting were added, including picture/cutscene frames. Syntax-only Switch and
desktop platform checks pass; this is still not a built or console-validated game.

## night100 (PS2 native port)

[night100](https://github.com/matthewperiut/night100) is a useful PS2 reference,
but uses Heavy Iron/RenderWare rather than Ratchet's engine. Its
[src/graphics/deko3d_backend.cpp](https://github.com/matthewperiut/night100/blob/main/src/graphics/deko3d_backend.cpp)
retains geometry and per-frame upload allocations, fences slot reuse, defers
texture descriptor retirement, and records GPU timestamps. Its fixed Switch
shader catalog is compiled ahead of time. These resource-lifetime and diagnostic
patterns are applicable; guest addresses, model formats and IOP worker hooks
require game-specific analysis. OpenRAC already does GPU skinning in
`port/viewer/shaders/mesh.vert`; moving skinning to the GPU is not a new win here.
A native texture/material bridge remains necessary before adapting its model and
particle approach to our Deko3D game renderer.

## SwitchWaker

[SwitchWaker](https://github.com/centollOS/SwitchWaker) adds a native Deko3D
backend under Aurora, with WGSL-to-GLSL translation and a patched UAM compiler.
Its shader/cache architecture is useful, but its shader inputs and bindings do
not match OpenRAC. Its SDK patch notes flag command-buffer clear control-memory
leaks and separate-alpha blend-factor handling in Deko3D 0.5. These findings
require targeted validation; the installed SDK has not been modified.

Applied to our prototype: frame slots now retain recorded command lists for
upload, clear, triangle and indexed-mesh draws. Upload pixels and transform data
still update only after the slot fence. Command kind, readback mode, clear color
and geometry generation determine reuse. Replacing geometry invalidates every
slot's list after waiting for the old geometry's consumers. Changed recordings
destroy/recreate their command-buffer owner instead of repeatedly calling
`dkCmdBufClear`. This avoids that reported clear path and reduces repeated CPU
recording for the current fixed draws. It is not a general scene batching system
or a measured performance claim. No SDK patch or reference-game code was copied.

## 2026-10-29: dkgl graphics-correctness review

The hardware-tested dkgl renderer is separate from the earlier Presenter probe.
Its SDK vertex-buffer command reservation workaround remains in place; the next
hardware run reported no new GPU errors. Visual defects remain under test.

### night100 and SwitchWaker

The current [night100 backend](https://github.com/matthewperiut/night100/blob/main/src/graphics/deko3d_backend.cpp)
uses clockwise rasterizer front faces and a public command capture/replay
workaround for Deko3D 0.5's separate-alpha destination-factor defect. Its cached
blend-command approach is applicable to dkgl. The
[SwitchWaker SDK patch notes](https://github.com/centollOS/SwitchWaker/blob/main/switch/deko3d/PATCHES.md)
also document that alpha bug and a control-memory leak; neither its reported
performance nor its game-specific bindings are OpenRAC results.

Applied source changes:
- Compensate the additional vertex Y flip performed by dk_pack.py in rasterizer
  front-face selection. Deko3D 0.5's viewport already compensates its own origin
  in face classification. Preserve front/back identity rather than disabling
  culling or changing all shader coordinates. Hardware image confirmation pending.
- Capture and validate the SDK's seven-word blend packet, correct only the
  destination-alpha factor, then cache/replay by all six blend fields. Accept
  both old and corrected SDK payloads; reject unexpected packet layouts visibly.
  This leaves the installed SDK untouched. No reference port's full renderer is
  copied, and the packet generation follows the pinned SDK's own implementation.
- Publish/invalidate newly created sampler descriptors before the same draw:
  previously unit_handle could write descriptors after cmd() had already checked
  the dirty flag, delaying invalidation until the following draw.

### NetherSX2_nx

Inspected revision `a78cb5836adb9024e6b0cd9b5860a526d4dda68b` of
[NetherSX2_nx](https://github.com/NaGaa95/NetherSX2_nx).
Its `dk_render_state.h` chooses OriginLowerLeft + DepthZeroToOne and the last
provoking vertex; dkgl already preserves last-vertex semantics, but uses an
UpperLeft device with explicitly flipped vertex shaders. Copying the other
port's origin flags would change our shader/framebuffer convention incorrectly.
Its packed PS2 GS vertex format also differs from OpenRAC's native scene format.

Its [transfer checks](https://github.com/NaGaa95/NetherSX2_nx/blob/a78cb5836adb9024e6b0cd9b5860a526d4dda68b/source/deko3d/dk_transfer.h)
and texture-rectangle validation are useful engine-independent patterns. Added
wide-arithmetic row-pitch/ring-size checks and mip-rectangle bounds before dkgl
issues texture DMA commands. Unsupported client/internal format conversions now
report an error rather than silently filling textures with zero. Float RGBA
joint-palette staging is covered by the host size tests. These are validation
improvements, not proof that the palette or vendor preview bug is fixed.

The mip-generation and image-visibility code was also compared. dkgl already has
per-mip waits and post-copy invalidation, so no duplicate barriers were added.
NetherSX2 replaces callbacks in a binary Android emulator core; those address-
specific hooks and runtime compiler/LSFG pipeline are not replacements for this
native game's dkgl integration. No frame-generation work is included here.

Host tests for winding, separate alpha across all supported factors, blend-cache
keys, unexpected SDK packets, padded texture rows, mip bounds and overflow pass.
The command-buffer crash regression also passes. These tests model CPU contracts;
they do not validate GPU output. Test Novalis, landing ship, title/menu Ratchet
and vendor preview on Switch before declaring visual correctness.
