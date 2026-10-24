# FSR temporal upscaling

Enhanced Vulkan rendering uses AMD FSR 3.1.4 to reconstruct the world before drawing the HUD at
display resolution. It includes temporal upscaling and sharpening, with no frame generation.

## Interface

- `fsr3.h`: frontend-owned upscaler, displayed poses and synchronous frame hooks.
- `sdk_context.h`: SDK context and imported colour, float depth, motion and reactive images.
- `temporal_math.h`: render extents and camera inversion.

The frontend requires enhanced graphics and compatible probe data. OpenGL, unsupported devices
and builds without the upscaler retain native rendering.

## Depends on

The renderer, scene, Vulkan backend and MIT-licensed AMD FidelityFX SDK 1.1.4. CMake pins commit
`c6efa6bf7f2027b3ec94f28578bb5965eabb9e55` and verifies the download hash. Existing glslang compiles
the SDK's GLSL; no extra shader compiler is required. SDK files stay in the build tree and are
not needed at runtime.

`prepare_sdk.py` embeds the shader dependency closure and corrects Vulkan integration details:
the core memory-requirements entry point, unified-memory allocation requiring all requested flags,
the FP16 luma-history image declaration, and the unused frame-generation swapchain callback.
AMD licence notices remain in generated sources.
The generated upscaler header also sizes context storage for the platform's wide characters;
the SDK and its callers use this same header. `sdk_compat.h` supplies missing standard includes
and bounded diagnostic formatting on non-Windows platforms.

## State and threads

One `Fsr3` belongs to the frontend's graphics owner: the window thread in synchronous mode,
or the worker during world recording. It owns world colour and depth, velocity and reactivity
targets, SDK histories and previous displayed camera/object poses.
The renderer lends its existing command buffer and images. There are no per-frame CPU readbacks,
extra queue submissions or GPU waits. Reconfiguration and destruction wait before freeing SDK data.

Static geometry and sky use depth-derived camera motion. Opaque live mobys overwrite it using
current and previous rigid transforms and skinned joints. Material alpha determines coverage.
A preparticle colour snapshot supplies reactivity for blended and additive effects.

History resets after resolution or level changes, skipped world frames, large camera changes and
explicit invalidation. New or teleported objects reject old history. The HUD and menu snapshot
follow reconstruction. The saved MSAA choice is restored when FSR is inactive.

## Errors

Setup returns `false` with a diagnostic. A dispatch failure presents a spatially scaled world and
native HUD, logs the error, and stops further temporal dispatches until the upscaler is recreated.
Unsupported builds expose no active FSR mode and allocate no SDK resources.

## Configuration

`OPENRAC_FSR3` builds the SDK only with `OPENRAC_ENHANCED_RENDERING` and `OPENRAC_VULKAN`. It defaults
to the enhanced build option. Set it to `OFF` to omit the dependency. For offline builds,
`FETCHCONTENT_SOURCE_DIR_FSR3_SDK` names an extracted copy of the pinned archive.

The Enhanced menu and launcher expose `temporal_quality`: `off`, `native-aa`, `quality` (default),
`balanced`, or `performance`. Display-to-render ratios are respectively 1x, 1x, 1.5x, 1.7x and 2x,
within the existing render-scale setting. `temporal_sharpness` ranges from 0 to 1, default 0.2.
See [settings](../../../docs/port/SETTINGS.md) for the enhanced master switches.

## Tests

- `fsr3_shaders`: supported FP32 shader variants and descriptor reflection.
- `fsr3_sdk`: uniform RGB preservation and history reset on the GPU.
- `fsr3_frame`: preset extents, camera inversion and native-pixel HUD coverage.
- `fsr3_motion`: rigid/skinned velocities, history rejection and exact batched/individual coverage,
  including alpha cutouts and glow exclusion.
- `python -m unittest discover -s port/enhanced/fsr3`: source embedding and repeatable preparation.

GPU fixtures use Vulkan validation and no game data.

## Limitations

Input colour is LDR. Translucent objects and particles have reactive coverage rather than individual
motion vectors. Animated sky shells use camera motion. Dynamic velocities upload current and previous
poses together, then instance consecutive objects sharing one geometry/material run. Multi-material
objects retain instance-major ordering to preserve equal-depth alpha coverage. The individual-draw
reference is the registry's `fsr-motion-batch` switched off (`OPENRAC_RENDER_OPT_OFF=fsr-motion-batch`,
[RENDER_PERF.md](../../../docs/port/RENDER_PERF.md#fsr-motion-batch-object-motion-in-batches)).

## Further reading

[AMD's integration guide](https://gpuopen.com/manuals/fidelityfx_sdk/techniques/super-resolution-upscaler/).
