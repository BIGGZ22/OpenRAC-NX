# What drawing costs: the counters and the optimisation registry

The port draws every level at native resolution with every upscaled feature, in full detail. The
render packages of the restructure (phase R, [PROGRESS.md](../restructure/PROGRESS.md)) lower what
drawing that same picture costs; none of them changes a pixel unless its entry below states a
tolerance the maintainers accepted, and none brings back the PS2's level of detail, draw distance or
fog distance, adds a placeholder or a skipped draw, or delays a visible update.

Two rules decide whether such a change pays:

- **Counts and bytes decide, milliseconds inform.** The counters below are exact: the same work
  gives the same numbers on any machine. A package's pass or fail is the counter it names; the
  milliseconds are reported beside it, as a median of at least five runs with the spread.
- **Every optimisation can be switched off.** Each one is an entry of the registry, so its
  reference path stays and can be compared with it frame by frame.

## The optimisation registry

[`port/renderer/render_opt.h`](../../port/renderer/render_opt.h) holds one table of every render
optimisation: its name, what it makes cheaper, whether it is on by default, and its tolerance.

- The draw code asks `render_opt::enabled(Opt::...)` and takes the reference path (the code as it
  was before the optimisation) when it is off. The question is one array read after the first call,
  so a draw may ask it each time.
- `OPENRAC_RENDER_OPT_OFF=name,name` switches the named entries off for a run, and
  `OPENRAC_RENDER_OPT_OFF=all` every one. Spaces around a name and empty items are ignored; a name
  that is no entry's is logged as a warning and switches nothing. The variable is read once, at the
  first question ([DEBUGGING.md](DEBUGGING.md)).
- `render_opt::set` and `render_opt::reset` switch entries between frames for the tests and the
  benchmark.
- The tolerance is zero (byte-identical frames) unless the entry says how many of a frame's pixels
  may differ and by how much in a channel. A tolerance is the maintainers' to accept, in the
  package's report.

The twelve older `*_OFF` switches of [RAC1_GPU_DRIVEN.md](RAC1_GPU_DRIVEN.md) stay what they are;
no optimisation adds an environment variable of its own.

A new entry is an enumerator of `render_opt::Opt` and a row of the table in `render_opt.cpp` at the
same place, and a section on this page: what it changes, the counter it lowers with its numbers
before and after, and how its frames are shown to be the same.

| Name | Default | Tolerance | What it does |
|---|---|---|---|
| `registry-selftest` | on | none (exact) | Draws nothing. The entry the registry's own tests and the equivalence harness's switch. |

Its tests are `render_opt` (the table, and the parsing of the variable: a name, a list, `all`, an
unknown name kept apart; it runs in the headless preset too) and `render_opt_environment` (the
variable as ctest sets it, read and applied, with its warning).

## The counters

The Vulkan backend counts what it records as it records it, into `vkgl::SubmissionCounters`
([`vkgl.h`](../../port/renderer/vulkan/vkgl.h)): cumulative from the moment a context opens, read
by a test or the benchmark with `vkgl::submission_counters()`. Each count is an add where the work
happens, with no clock, no allocation and no test of whether anyone is looking. Once per frame,
`present()` publishes them as gauges of the frame (`stats::gauge`), which the F3 overlay shows and
`--debug-dump` writes ([DEBUGGING.md](DEBUGGING.md), "The frame dump"); that publication is the one
place that asks whether the debug tools are measuring.

A gauge shows the frame's own part of a counter, or, where the table says "total", the counter as
it stands. The last column names the package of phase R whose result the counter decides.

| Gauge | Frame or total | What it counts | Counted in | Decides |
|---|---|---|---|---|
| `vk_cmds_recorded` | frame | Every `vkCmd*` recorded, the kinds below added up | each call site | R-03 |
| `vk_cmds_bind_pipeline` | frame | `vkCmdBindPipeline` | `vkgl_draw.cpp`, `vkgl_pipelines.cpp`, `gpu_draw.inc` | R-03 |
| `vk_cmds_bind_descriptors` | frame | `vkCmdBindDescriptorSets` and `vkCmdPushConstants` | the same | R-03, R-14 |
| `vk_cmds_bind_buffers` | frame | `vkCmdBindVertexBuffers` and `vkCmdBindIndexBuffer` | `vkgl_draw.cpp` | R-03 |
| `vk_cmds_dynamic_state` | frame | `vkCmdSetViewport`, `vkCmdSetScissor`, `vkCmdSetBlendConstants`, the three `vkCmdSetStencil*` | `vkgl_draw.cpp` | R-03, R-11 |
| `vk_cmds_draw` | frame | `vkCmdDraw`, `vkCmdDrawIndexed`, `vkCmdDrawIndexedIndirect` | `vkgl_draw.cpp`, `vkgl_commands.cpp` | R-03 |
| `vk_cmds_barrier` | frame | `vkCmdPipelineBarrier` | every barrier | R-08 |
| `vk_cmds_copy` | frame | `vkCmdCopy*`, `vkCmdBlitImage`, `vkCmdResolveImage` | `vkgl_resources.cpp`, `vkgl_gl.cpp`, `vkgl_present.cpp`, `gpu_draw.inc` | R-07 |
| `vk_cmds_dispatch` | frame | `vkCmdDispatch` | `vkgl_pipelines.cpp`, `gpu_draw.inc` | |
| `vk_cmds_render_pass` | frame | `vkCmdBeginRendering` and `vkCmdEndRendering` | `vkgl_commands.cpp`, `vkgl_gl.cpp` | R-06 |
| `vk_cmds_query` | frame | `vkCmdWriteTimestamp`, `vkCmdResetQueryPool` (the GPU timers) | `vkgl_commands.cpp` | |
| `vk_cmds_other` | frame | `vkCmdClearAttachments` and the debug labels | `vkgl_commands.cpp` | |
| `vk_pipelines_created` | total | Graphics and compute pipelines made, each a compile in the driver | `vkgl_pipelines.cpp`, `vkgl_gl.cpp`, `gpu_draw.inc` | R-10, R-11 |
| `vk_pipelines_created_after_warmup` | total | Of those, the ones made after the warm-up `vkgl::begin_pipeline_warmup(N)` started (N frames after the benchmark's level upload); 0 until it is called | the same | R-10 |
| `vk_descriptor_sets_allocated` | frame | Descriptor sets allocated, the material pages' included | `vkgl_pipelines.cpp`, `gpu_materials.inc` | R-14 |
| `vk_descriptor_writes` | frame | `VkWriteDescriptorSet` entries written | `vkgl_pipelines.cpp`, `gpu_materials.inc`, `gpu_draw.inc` | R-14 |
| `vk_render_passes` | frame | Render passes begun | `vkgl_commands.cpp`, `vkgl_gl.cpp` (a depth resolve) | R-06, R-07 |
| `vk_attachment_load_bytes` | frame | Bytes the passes read in: each attachment whose load op is LOAD, its texel bytes times the render area times its samples (a depth and stencil image once, loaded when either aspect is) | `vkgl_commands.cpp`, `vkgl_gl.cpp` | R-06 |
| `vk_attachment_store_bytes` | frame | Bytes written back: each attachment whose store op is STORE, and a resolve's target, measured the same way | the same | R-06 |
| `vk_barriers` | frame | Pipeline barriers recorded | every barrier | R-08 |
| `vk_barriers_all_commands` | frame | Of those, the ones with `ALL_COMMANDS` as their source or destination stage | every barrier | R-08 |
| `vk_submits` | frame | `vkQueueSubmit`: one per frame, and one per read-back (`glReadPixels`, `glFinish`) | `vkgl_commands.cpp` | R-08 |
| `vk_frames_presented` | total | Frames ended by `present()` | `vkgl_present.cpp` | |
| `vk_fullscreen_copies` | frame | Copies, blits and resolves whose destination is the default framebuffer's size or the screen's | each copy | R-07 |
| `vk_ring_bytes` | frame | Bytes taken from the frame's upload ring (vertices, indices, uniforms, texels); the older `vk_ring_kb` is the same in KiB | `vkgl_commands.cpp` | R-05, R-15 |
| `vk_texture_creations` | frame | Images made: textures, render targets, the default framebuffer, mip chains made again | `vkgl_resources.cpp` | R-13 |
| `vk_texture_upload_bytes` | frame | Texel bytes staged for upload into images | `vkgl_resources.cpp` | R-13 |
| `vk_buffer_static_bytes_host_visible` | total | Bytes of the buffers that stopped changing and got memory of their own, where that memory is host-visible (`vmaGetAllocationMemoryProperties`) | `vkgl_resources.cpp` | R-05 |
| `vk_cpu_mirror_bytes` | total | The bytes of the CPU copies the backend keeps of GL's buffers, now | `vkgl_gl.cpp` | R-05 |
| `vk_gpu_payload_bytes` | frame | Bytes the CPU writes for the GPU-driven instances: each batch's view and instances as `gpu_make_batch` builds them, and the page header, groups, commands and payload `gpu_record` copies into the mapped input page | `gpu_draw.inc` | R-15 |
| `vk_shader_compiles` | total | glslang runs: each shader checked at `glCompileShader`, each program compiled at `glLinkProgram`, the GPU-driven and material programs | `shader_compiler.cpp` | R-12 |
| `shader_compile_s` | total | The seconds those runs took | `shader_compiler.cpp` | R-12 (informs) |

The gauges the backend published before (`vk_draws`, `vk_draw_api_calls`, `vk_pipelines_made`,
`vk_ring_kb`, `vk_uniform_*`, `vk_descriptor_sets`, `vk_gpu_*`, `vk_draw_ms`, `vk_present_ms`,
`vk_wait_ms`) are unchanged.

`render_opt_vulkan` draws a fixed scene (six full-screen triangles a frame for three frames, on the
64 by 64 default framebuffer of a hidden window) and checks that the counters follow: every draw is
a recorded draw command, the commands recorded are at least the draws, and each frame without a
read-back is one submit. It prints the counters, as the section on R-01 below quotes them.
