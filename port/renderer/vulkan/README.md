# renderer/vulkan

The port's Vulkan renderer: the short list of OpenGL calls every renderer is written against
(`renderer/gl.h`, `renderer/world/gl_world.h`) carried out with Vulkan 1.3, so that the renderers,
their shaders and the frontend run on it unchanged. It is chosen at start with
`OPENRAC_GL_API=vulkan` (`platform/window.cpp`), and it is what draws on a player's own Vulkan
driver on Android. It is not a general OpenGL implementation: a GL call the renderers do not make is
not here, and a new renderer feature has to be expressible in that list. How GL's model becomes
Vulkan's is the comment at the top of [vkgl.h](vkgl.h).

## Interface

- [vkgl.h](vkgl.h): `openrac::vkgl`, the backend as the window and the frontend see it: `open` and
  `close`, `get_proc` (the GL functions by name, for `renderer/gl.h`'s table), `present`, the swap
  interval, the GPU-driven instance calls and the counters the debug tools read.
- [shader_compiler.h](shader_compiler.h): GLSL made into SPIR-V with glslang, with what an OpenGL
  program would answer about its uniforms and blocks.

The rest is the backend's own, which nothing outside this directory includes:

| File | What it holds |
|---|---|
| [vkgl_internal.h](vkgl_internal.h) | The types, constants and state (`Context`, the one `g`) the files share, and the functions each file offers the others, grouped by the file that defines them |
| [vkgl.cpp](vkgl.cpp) | `g`, `open`, `close`, `present` and the rest of `vkgl.h` |
| [vkgl_commands.cpp](vkgl_commands.cpp) | Work deferred to the frame's end, the frame's ring, the two command buffers (`upload`, `main`) and the order between them, GPU timing, submission, render targets and `glClear` |
| [vkgl_resources.cpp](vkgl_resources.cpp) | Images, views, pixel uploads and mip chains, samplers, buffers on the GPU and their texel views |
| [vkgl_pipelines.cpp](vkgl_pipelines.cpp) | Vertex formats, fixed-function state, pipelines by key, descriptor sets, `glDispatchCompute` and `glMemoryBarrier` |
| [vkgl_draw.cpp](vkgl_draw.cpp) | One draw recorded with the current state; includes the two files below |
| [gpu_materials.inc](gpu_materials.inc) | The native material path: material records and textures indexed by the draw |
| [gpu_draw.inc](gpu_draw.inc) | GPU-driven instances: culling on the GPU and indirect draws |
| [vkgl_gl.cpp](vkgl_gl.cpp) | The GL functions themselves (state, textures, buffers, vertex arrays, shaders and programs, uniforms, samplers, framebuffers) and their table |
| [vkgl_present.cpp](vkgl_present.cpp) | The default framebuffer, the swapchain and its present mode, moving on to the next frame, presenting |
| [vkgl_device.cpp](vkgl_device.cpp) | The instance and the validation layer, the physical and logical device, the pipeline cache on disk, the frames' objects, the dummy resources, the surface, and teardown |
| [pipeline_cache_file.h](pipeline_cache_file.h), [.cpp](pipeline_cache_file.cpp) | The pipeline cache's file: the driver's data in an envelope with its length and hash, read back only when whole, saved through a temporary file of each save's own and renamed into place |
| [vma.cpp](vma.cpp) | The Vulkan Memory Allocator's implementation, compiled once |

## Depends on

`renderer` (the GL interface it carries out, `renderer/gpu_draw.h`), `renderer/world`
(`gl_world.h`'s calls and enums) and `common` (the log, the statistics, `float_encoding.h`). Below
them, SDL3 for the window's surface. The third-party libraries it wraps: volk (Vulkan's functions),
the Vulkan Memory Allocator, and glslang (in `shader_compiler.cpp` only). The CMake target is
`openrac_renderer_vulkan` (`port/cmake/Window.cmake`).

## State and threads

All of the backend's state is one `Context`, reached through `detail::g` (`vkgl_internal.h`): made
by `open()` and destroyed by `close()`, null while the backend is closed. Only one backend is open
at a time, used from the thread that opened it, as the renderers use GL. The one exception is
`on_event`, which SDL calls from its event thread when an Android app returns to the foreground; it
only sets `surface_lost`, which is atomic. Two frames are in flight: each frame slot's objects are
reused, and its deferred work run, once its fence has signalled. A static buffer of 64 KiB or more
lives in device memory from its first draw and its CPU copy is let go; a reader of its bytes reads
them back with a command buffer of its own (`buffer_bytes`,
[RENDER_PERF.md](../../../docs/port/RENDER_PERF.md), "R-05"). Turning `g` into an object the
backend passes around is P3-03 of the restructure
([PROGRESS.md](../../../docs/restructure/PROGRESS.md)).

`Context::recorded` (a `RecordedState`) is what the frame's main command buffer holds of a draw's
state: the viewports, scissors, blend constants and stencil states last set, and the descriptor
sets with their layout and offset, the push constant, the vertex buffers and the index buffer last
bound. A draw records only what differs from it (the registry's `vk-state-skip`); the pipeline is
`Context::bound_pipeline`. It is forgotten at every begin of rendering and of the command buffers,
so nothing is trusted across passes, and a change of pipeline layout forgets the sets. Code that
records graphics state or binds into `main` outside `draw()` must forget it too.

The draw finds its pipeline in `Context::pipelines`, a `PipelineTable` (each pipeline once, found
by a hash of its key's bytes and a compare of the whole key, or by the key as a string on the
reference path), and its descriptor set in `Program::set_cache`; its temporary arrays are
`Context::scratch`'s, kept for their capacity, so a draw that finds both allocates nothing (the
registry's `vk-draw-cache`). A deleted program's pipelines leave the table with it.

## Errors

`open()` returns false with a message when Vulkan cannot start (the window then falls back to
OpenGL and logs why). After that, a failing Vulkan call is logged (`check()`, `vulkan: <what>
failed`) and the work that needed it is skipped; GL errors are recorded for `glGetError` as GL
would. The GPU-driven path throws `std::runtime_error` when a page's fixed capacity is exceeded.
With the validation layer on, its messages go to the log and its errors are counted.
The pipeline cache's file reports a damaged file in its `SavedCache` and a failed save as false
with a message (`pipeline_cache_file.h`); `vkgl_device.cpp` logs both as warnings, and the run goes
on building its pipelines afresh.

## Configuration

Read at start. [DEBUGGING.md](../../../docs/port/DEBUGGING.md) lists them, except the two
GPU-driven references, which [RAC1_GPU_DRIVEN.md](../../../docs/port/RAC1_GPU_DRIVEN.md) lists:

- `OPENRAC_VULKAN_VALIDATION` (`vkgl.cpp`): Khronos' validation layer.
- `OPENRAC_VULKAN_PIPELINE_CACHE` (`vkgl_device.cpp`): where pipelines are cached between runs,
  or off. The cache file is named after a key that hashes this directory's sources
  (`OPENRAC_VK_PIPELINE_KEY`, `port/cmake/Window.cmake`), so a build whose pipeline code changed
  starts a fresh cache. Programs that save the same file at once (tests run in parallel) each
  write their own temporary file and rename it into place, and a file that is not one whole save
  is dropped on reading (`pipeline_cache_file.h`): a damaged cache once drew wrong colours.
- `OPENRAC_VK_UNIFORM_REUSE_OFF` (`vkgl_internal.h`), `OPENRAC_GPU_DRIVEN_OFF` (`vkgl.cpp`),
  `OPENRAC_GPU_MATERIALS_OFF` (`gpu_materials.inc`): the reference paths the faster ones are
  measured against, as statistics probes.
- `OPENRAC_GPU_MARKERS` (`vkgl_commands.cpp`, `vkgl_device.cpp`): names the timed GPU passes for
  profilers, in a build configured with `-DOPENRAC_GPU_MARKERS=ON`.

## Tests

The renderer's tests run again on this backend under `_vulkan` names (`port/cmake/Window.cmake`),
with `OPENRAC_GL_API=vulkan` and the validation layer where it is installed (Mesa's lavapipe in
CI). Where Vulkan cannot open, the window falls back to OpenGL and they repeat the OpenGL runs.

- `renderer_direct_vulkan`, `renderer_shader_vulkan`, `touch_overlay_vulkan`, `imgui_gl_vulkan`,
  `world_gl_smoke_vulkan`, `viewer_vulkan`: every GL call the renderers make, carried out here.
- `vulkan_uniform`, `vulkan_uniform_reference`: uniform uploads and descriptor reuse, with the
  reuse on and off (`OPENRAC_VK_UNIFORM_REUSE_OFF`); a state bounce (A, B, A) draws A's pipeline
  and set again, and deleting one program's pipelines leaves the others' usable.
- `render_opt_vulkan`: the render counters of `SubmissionCounters` follow a fixed scene's draws
  (every draw a draw command, one submit a frame), printed for the reports of
  [RENDER_PERF.md](../../../docs/port/RENDER_PERF.md).
- `vulkan_buffer`: a static GL buffer drawn, rewritten in part, filled again, read through 8-bit
  indices and as a uniform block, frame after frame, under the reference path, device memory and
  the staging copy (`static-buffers-*`); the mirror let go and nothing through the ring.
- `pipeline_cache_file`: the cache's file read back whole, a damaged one dropped, and writer threads
  and writer processes saving one file at once leaving one whole save. It needs no GPU.
- `early_lighting_vulkan`, `probe_gpu_vulkan`, `hero_shadow_vulkan`, `sun_shadow_vulkan`,
  `sun_shadow_filter_reference_vulkan`, `gtao_vulkan`, `gtao_horizon_vulkan`,
  `enhanced_rendering_vulkan`, `enhanced_rendering_off_vulkan`: the enhanced rendering's passes on
  Vulkan.

## Further reading

- [vkgl.h](vkgl.h): how GL's objects, programs, state and rows map onto Vulkan.
- [docs/port/ANDROID.md](../../../docs/port/ANDROID.md): why the port has a Vulkan renderer, and
  the custom drivers it serves.
- [docs/port/RAC1_GPU_DRIVEN.md](../../../docs/port/RAC1_GPU_DRIVEN.md): the GPU-driven instances
  and the native material path.
- [docs/port/DEBUGGING.md](../../../docs/port/DEBUGGING.md): the variables above, and the
  counters (`vkgl::submission_counters()`).
- [docs/port/RENDER_PERF.md](../../../docs/port/RENDER_PERF.md): every render counter and gauge,
  where it is counted, the timed sections, and the optimisation registry.
