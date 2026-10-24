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

## Time per pass

Every pass the frame runs is a `stats::Scope` ([common/stats.h](../../port/common/stats.h)): CPU
time always, GPU time as well (`stats::kGpu`) where the backend has timestamp queries. Each
timed section is a row of the F3 overlay and of each frame in `--debug-dump`. The milliseconds
inform; on a shared machine they are noise beside the counters.

When a context opens, the Vulkan backend reads its queue's `timestampValidBits` and the device's
`timestampPeriod` and logs one line, `vulkan: gpu timestamps: yes (64 valid bits, 1 ns a tick)` or
`vulkan: gpu timestamps: no (...)`; the gauge `vk_gpu_timestamps` is 1 or 0 each frame. With no
timestamps the GPU columns stay empty and the CPU columns are still there.

The sections, by where they are taken:

| Where | Sections | GPU time |
|---|---|---|
| The frontend (`game/common/frontend.cpp`) | `render`, `swap` | no |
| `Renderer::render` (`renderer/renderer.cpp`) | `frame clear`, `world depth prepass`, `probe coverage selection`, `world opaque colour`, one per bucket renderer (its name), `texture retire` | the prepass, selection and opaque colour, and each renderer that does not time itself; `frame clear` and `texture retire` are CPU work |
| GTAO (`enhanced/renderer/gtao.cpp`) | `gtao`, `GTAO depth copy`, `GTAO horizons`, `GTAO filter` | yes, but `gtao` |
| Early lighting (`enhanced/viewer/early_lighting.cpp`) | `early GI and cascades` | yes |
| `LevelScene` (`scene/level_renderer.cpp`) | `shadow atlas`, `sun0 map` to `sun3 map`, `Ratchet shadow`, `shadow clear`, `sky colour`, one per layer | yes, as `OPENRAC_SUN_TIMING_DETAIL` chooses for the cascades |
| The GPU-driven culling (`renderer/vulkan/gpu_draw.inc`) | `GPU instance visibility` | yes |
| The Vulkan present (`renderer/vulkan/vkgl_present.cpp`) | `vulkan present` (acquire, the copy to the screen, submit, present), `vulkan frame wait` (the fence of the frame two back) | no |

`frame clear`, `texture retire`, `vulkan present` and `vulkan frame wait` came with R-01; the rest
were there. Not timed on their own yet, because their files belong to other packages:
`LevelScene::upload` (a level's upload, at the level's load), and `LevelScene::set_palette` and
`set_instances` with its `build_groups` (the moby joint palette and instance groups, which the
frontend's `draw_kept` hands the scene before its `render` section opens), so no section holds their
time. `LevelScene::prepare_shadows` (the cascades' culling on the CPU) runs inside the pass that
calls it and is counted in that pass's time. PROGRESS.md's follow-ups list them for the scene's
packages.

## R-01: the counters, the timers and the registry

**What changed.** The registry (`renderer/render_opt.h`, the entry `registry-selftest`), the
counters of `vkgl::SubmissionCounters` and their gauges, the four CPU sections above and the
timestamp line. No draw, submit or image changes: every count is an add beside the call it counts,
and the gauges are written only while the debug tools measure.

**Equivalence.** Nothing draws differently, so the proof is the existing image tests passing
unchanged: on the Mac every Vulkan test of the release preset (`viewer_vulkan`, whose fixtures
compare CPU and GPU frames byte for byte with their draw and triangle counts, `vulkan_uniform`,
`vulkan_uniform_reference`, `renderer_*_vulkan`, `sun_shadow_*`, `hero_shadow_vulkan`, `gtao_*`,
`early_lighting_vulkan`, `probe_gpu_vulkan`, `enhanced_rendering_*`), and the same again with
`OPENRAC_RENDER_OPT_OFF=all`; on box1 the whole release preset, the OpenGL tests included (they
skip on the Mac, which has no GL context in an agent's session).

**The counters on `render_opt_vulkan`'s scene** (six triangles a frame, three frames, 64 by 64).
Before R-01 none of these existed; the older gauges are unchanged. The counts are the same on both
machines; the attachment bytes differ because each device has its own depth format
(`VK_FORMAT_D32_SFLOAT_S8_UINT`, 8 bytes, on MoltenVK; `VK_FORMAT_D24_UNORM_S8_UINT`, 4 bytes, on
the NVIDIA driver), so a counter is compared on one machine, before against after.

| Counter | Mac (M5 Max, MoltenVK 1.4.2, macOS 27.0) | box1 (GTX 1070, NVIDIA 580.178.04, Vulkan 1.4.312, Manjaro) |
|---|---|---|
| `vk_cmds_recorded` | 165 | 165 |
| `vk_cmds_draw` | 18 | 18 |
| `vk_cmds_dynamic_state` | 108 | 108 |
| `vk_cmds_bind_descriptors` | 18 | 18 |
| `vk_cmds_bind_pipeline` | 3 | 3 |
| `vk_cmds_barrier` | 9 | 9 |
| `vk_cmds_render_pass` | 6 | 6 |
| `vk_cmds_query` | 3 | 3 |
| `vk_render_passes` | 3 | 3 |
| `vk_attachment_load_bytes` | 98,304 | 49,152 |
| `vk_attachment_store_bytes` | 147,456 | 98,304 |
| `vk_barriers`, of them `ALL_COMMANDS` | 9, 9 | 9, 9 |
| `vk_submits` | 3 | 3 |
| `vk_descriptor_sets_allocated`, `vk_descriptor_writes` | 3, 0 | 3, 0 |
| `vk_pipelines_created` (total) | 1 | 1 |
| `vk_texture_creations` (total: the dummy image, the default colour and depth) | 3 | 3 |
| `vk_shader_compiles` (total) | 3 | 3 |

Already visible: of 165 commands for 18 draws, 108 are dynamic state set again before every draw
(six a draw) and 18 descriptor binds of an unchanged set, which is what R-03 removes.

**Milliseconds, as information.** The five image tests `vulkan_uniform`, `viewer_vulkan`,
`renderer_shader_vulkan`, `renderer_direct_vulkan` and `world_gl_smoke_vulkan`, one after another,
five runs each (the first run on the Mac starts with a cold pipeline cache): on the Mac 0.93 s
before and 0.93 s after (median; 0.91 to 2.05 and 0.91 to 1.95), on box1 1.46 s before and 1.44 s
after (1.45 to 1.49 and 1.43 to 1.45). No change beyond the noise.

Measured with:

```sh
cd port && cmake --preset release && cmake --build --preset release
OPENRAC_GL_API=vulkan OPENRAC_VULKAN_VALIDATION=1 ./build/release/render_opt_test --vulkan
ctest --preset release -R '^(vulkan_uniform|viewer_vulkan|renderer_shader_vulkan|renderer_direct_vulkan|world_gl_smoke_vulkan)$'
```

The synthetic benchmark (`openrac_render_bench`, `port/tools/perf/render_bench.py`) and the
equivalence test come with R-02, which adds the baseline table and the commands for real levels.
