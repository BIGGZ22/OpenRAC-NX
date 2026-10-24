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
| `vk-draw-clock` | on | none (exact) | The Vulkan draw reads the clock for `vk_draw_ms` only while the debug tools measure (R-03). |
| `vk-state-skip` | on | none (exact) | The Vulkan draw records dynamic state and binds only when they differ from what its command buffer holds (R-03). |
| `static-buffers-device-local` | on | none (exact) | A static GL buffer of 64 KiB or more gets device memory at its first draw and lets its CPU copy go ([R-05](#r-05-static-buffers-in-device-memory)). |
| `static-buffers-force-staging` | off | none (exact) | With the entry above, fills that memory by a staging copy even where device memory is mappable, so the tests take that path on every machine ([R-05](#r-05-static-buffers-in-device-memory)). |
| `texture-prepare` | on | none (exact) | A level's textures are created, uploaded and mipped at its load (`LevelScene::upload`), not by the first draw that uses them ([R-13](#r-13-a-levels-textures-made-at-its-load)). |
| `vk-draw-cache` | on | none (exact) | The Vulkan draw finds its pipeline by a hashed fixed key and its descriptor set through arrays kept for their capacity, with no heap allocation ([R-04](#r-04-an-allocation-free-draw)). |
| `attachment-discard` | on | none (exact) | A Vulkan pass neither stores an attachment `glInvalidateFramebuffer` let go nor has the next pass load it ([R-06](#r-06-attachments-a-pass-does-not-keep)). |
| `gpu-resident-instances` | on | none (exact) | Static instances are registered in device memory at the level's upload; a view's GPU-driven batch of them sends only its planes, PVS and a group record ([R-15](#r-15-static-instances-resident-on-the-gpu)). |
| `shadow-static-cache` | off | none (exact) | A sun cascade's static casters are kept in a cache target and copied into the shadow atlas while the cascade's key is unchanged; only the mobys are drawn over them. Off: the copy costs more GPU time than it saves on both measured machines ([R-18](#r-18-a-static-shadow-cache-shipped-off)). |
| `present-single-copy` | on | none (exact) | The frontend's blit of the picture into the window waits and becomes one blit into the screen, where nothing else touches the window first ([R-07](#r-07-one-copy-to-the-screen)). |
| `vk-tight-sync` | on | none (exact) | Vulkan barriers name the stages and accesses they order, and only the copy to the screen waits for the swapchain image ([R-08](#r-08-barriers-that-name-their-stages)). |
| `pipeline-prewarm` | on | none (exact) | The pipelines earlier runs made are made again on a worker thread once their program links, from a manifest beside the pipeline cache ([R-10](#r-10-pipelines-made-ahead-on-a-worker)). |

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

The synthetic benchmark and the equivalence test, with the baseline table and the commands for
real levels, are R-02's, below.

## The synthetic scene, the benchmark and the equivalence test

Three test programs on one synthetic scene, all in a hidden window: nothing is shown, no picture is
written, no retail data is read.

**The scene** ([`port/tests/render_fixture.h`](../../port/tests/render_fixture.h)) is built in
memory from a seed, with no clock: 4,000 static instances (ties and shrubs) whose 70 meshes add up
to about 1,000,000 triangles, 140 materials (two to a mesh; every tenth a cutout, every
seventeenth blended), 600 RGBA8 textures of 32 to 256 texels a side with their mip chains, 300 live
mobys of 46 skinned classes with an 8-joint palette each (one in ten blended), terrain, a sun, and a
fifth of the static instances small (0.3 units) and 1,200 to 2,400 units past the end of the
camera's walk, under one pixel there at 1080p. It is drawn through `LevelScene` and the renderer's
buckets as the frontend's `draw_kept` draws a level: each frame the mobys' palette and instances
are handed to the scene (`set_palette`, `set_instances`) and the renderer runs. Between two ticks
the mobys are blended from one tick to the next, as the frontend blends them.

The scenarios (`--scenario`): `walk` (the camera walks corner to corner, the mobys move), `static-mobys`
(the camera still), `turn` (the camera turns on the spot), `between` (three pictures between two
ticks, a quarter of a tick apart), `reupload` (the level uploaded again halfway, as a level change
does). A later package adds a scenario when its change needs one.

**The benchmark**, `openrac_render_bench` ([`port/tests/render_bench.cpp`](../../port/tests/render_bench.cpp)),
draws a scenario at `--size` (1920x1080 by default) for `--frames` pictures after `--warmup`
pictures and prints one JSON line per frame (its sections with CPU and GPU milliseconds, its
counters, every gauge) and a summary line (the picture times' median, 95th percentile and maximum,
the first picture after the upload, and the counters of the load, the warm-up and the measured
frames). It counts the heap allocations its drawing thread makes inside each frame's render call
(`bench_heap_allocs`) by replacing the global `operator new` in that program only. Options:
`--enhanced on|off` (sun cascades and GTAO), `--gpu-driven on|off`, `--materials on|off`
(the native material path against the individual one), `--cache cold|warm` (a private pipeline
cache folder, removed at exit; warm draws the run twice in one process and reports the second),
`--opt-off LIST`, `--camera`, `--between K`, `--msaa N`, `--tris N`, `--seed N`, and `--level DIR`
for an extracted level. `render_bench_smoke` runs ten frames of it at 640x360.

**The runner**, [`port/tools/perf/render_bench.py`](../../port/tools/perf/render_bench.py), runs the
benchmark over a matrix, five times a setting, checks that the counters come out the same in every
run, and prints the counters per frame, the run totals and the milliseconds (the median of the runs,
lowest to highest). The reduced matrix (the default) is the base setting (walk, cold cache,
GPU-driven, materials) and one change of it at a time; `--full` is every combination of Enhanced,
GPU-driven, materials, cache and camera. Results go to `port/build/render-bench/` (ignored by git).
`--compare BEFORE AFTER` prints two saved results with a percentage per counter, and
`--compare-opt NAME` the matrix with registry entry NAME on and off, the table each later
package's report gives.

**The equivalence test**, `render_equivalence_vulkan`
([`port/tests/render_equivalence_test.cpp`](../../port/tests/render_equivalence_test.cpp)), draws
the scene at a tenth of its size at 320 by 180, eight frames a scenario, under the validation layer.
It first checks that the scene is drawn at all (half the first frame is over the clear colour),
then draws every scenario twice, Enhanced off and on, and requires identical frames: every later
proof rests on the driver drawing alike twice. Then each registry entry is drawn on and off in every
scenario and the frames are compared with the entry's tolerance, byte for byte unless it states one.
The self-test entry is compared as well, as the harness's check of itself: its reference path is in
the fixture (`SceneRenderer::selftest_reference_path`). The validation layer must report nothing.

Its own proof: on both machines every scenario drew identical frames twice in both modes. A
deliberate one-pixel change in the self-test's reference path (a 1 by 1 scissored clear), run and
then removed before the commit, made the test fail in all ten comparisons, each reporting one pixel
of 57,600 different.

## The baseline

`python3 port/tools/perf/render_bench.py --synthetic --reduced --sections`, release preset
(RelWithDebInfo), 1920x1080, 60 measured frames after 10, five runs a setting, on 2026-10-23:

- **Mac**: Apple M5 Max, macOS 27.0, MoltenVK 1.4.2 (Vulkan 1.4.357 in `vulkaninfo`).
- **box1**: NVIDIA GeForce GTX 1070, driver 580.178.04, Vulkan 1.4.312, Manjaro Linux (kernel 6.18).

The counts are the same on both machines and in every run, except where a device's own choices
enter: the attachment bytes (MoltenVK's depth format is `D32_SFLOAT_S8_UINT`, 8 bytes a texel;
NVIDIA's `D24_UNORM_S8_UINT`, 4) and the heap allocations (the C++ libraries differ: libc++ on the
Mac, libstdc++ on box1). Each counter is compared on one machine, before against after.

Per measured frame, both machines (Mac / box1 where they differ):

| Counter | base (walk, cold) | enhanced | gpu-driven off | materials off |
|---|---|---|---|---|
| draw calls (the renderer's) | 1,460 | 2,934 | 1,459.6 | 1,460 |
| triangles | 1,390,692 | 6,996,294 | 767,778 | 1,390,692 |
| `vk_cmds_recorded` | 174 | 287 | 17,554.6 | 14,654 |
| `vk_cmds_draw` | 10 | 16 | 1,459.6 | 1,460 |
| `vk_cmds_dynamic_state` | 60 | 96 | 8,757.8 | 8,760 |
| `vk_cmds_bind_descriptors` | 31 | 45 | 4,378.9 | 1,461 |
| `vk_render_passes` | 1 | 4 | 1 | 1 |
| `vk_attachment_store_bytes` | 24,883,200 / 16,588,800 | 187,895,808 / 125,263,872 | as base | as base |
| `vk_barriers`, of them `ALL_COMMANDS` | 10, 5 | 27, 22 | 5, 5 | 10, 5 |
| `vk_fullscreen_copies` | 0 | 2 | 0 | 0 |
| `vk_ring_bytes` | 169,984 | 181,096 | 482,102 | 175,504 |
| `vk_descriptor_sets_allocated`, `vk_descriptor_writes` | 5, 615 | 13, 676 | 4, 612 | 939, 3,755 |
| `vk_texture_upload_bytes` (the moby palette) | 163,840 | 163,840 | 163,840 | 163,840 |
| `vk_gpu_payload_bytes` | 2,839,984 | 9,824,307 | 0 | 2,839,984 |
| `bench_heap_allocs` | 7,103 / 7,118 | 16,191 / 16,231 | 7,102 / 7,117 | 12,787 / 12,802 |

Per run, both machines:

| Counter | base (walk, cold) | enhanced | warm cache | materials off |
|---|---|---|---|---|
| pipelines made in the load and warm-up | 7 | 16 | 4 | 6 |
| pipelines made in the measured frames | 0 | 0 | 0 | 0 |
| `vk_texture_creations` in the warm-up (each texture twice: its image, then its mip chain) | 1,201 | 1,208 | 1,201 | 1,201 |
| texel bytes uploaded in the warm-up | 58,949,632 | 58,949,636 | 58,949,632 | 58,949,632 |
| glslang runs in the load | 38 | 69 | 38 | 15 |

The static camera, the pictures between ticks and the half size give the base's counts, but the
attachment bytes at half size (a quarter).

Milliseconds, information only (median of five runs, lowest to highest):

| Timing | Mac base | Mac enhanced | box1 base | box1 enhanced |
|---|---|---|---|---|
| picture (CPU: the render call and present) | 0.81 (0.79 to 0.82) | 1.86 (1.85 to 1.87) | 1.10 (1.09 to 1.11) | 3.12 (3.12 to 3.17) |
| first picture after the upload | 71.9 (70.2 to 75.5) | 123.2 (122.9 to 124.3) | 116.0 (115.2 to 116.7) | 190.3 (189.3 to 1,613.9) |
| the upload | 108.8 (108.0 to 114.5) | 547.6 (542.3 to 578.2) | 185.1 (184.6 to 191.8) | 644.3 (639.1 to 653.5) |
| glslang in the load, seconds | 0.07 | 0.19 | 0.14 | 0.36 |

On a warm cache the first picture is 43.8 ms on the Mac and 95.0 ms on box1 (cold: 71.9 and 116.0).

GPU time per pass, box1 (median of the first run's frames; a dash where the pass is timed by its
parts): ties solid 0.092 ms, shrubs 0.091, terrain 0.078, mobys opaque 0.076, mobys glow 0.065,
GPU instance visibility 0.035; with Enhanced, the depth prepass 0.152, the shadow atlas 0.267,
early GI and cascades 0.135, world opaque colour 0.112, GTAO 1.30 (depth copy 0.378, horizons
0.530, filter 0.387). At half size (960x540) the world passes take the same GPU time (ties solid
0.090 against 0.092, mobys opaque 0.072 against 0.076): on this scene they are bound by vertices,
not by pixels. The frame wait is 0.001 ms in the base setting and 0.29 ms with Enhanced: the GPU
keeps up with the CPU except in Enhanced mode on box1. On the Mac the GPU column is only meaningful
for a pass of its own (the shadow atlas 0.162 ms, GTAO's horizons 0.138 and filter 0.119, the
culling 0.025): MoltenVK's timestamps inside one render pass read about 0.002 ms whatever the pass
draws, since a tiled GPU runs a pass's draws together.

CPU time per pass, Mac / box1 (base): mobys 0.357 / 0.640 ms, ties 0.086 / 0.143, shrubs 0.072 /
0.112, terrain 0.004 / 0.009, GPU instance visibility 0.018 / 0.124.

### Which numbers decide which gate

- **G1** (R-22, R-23, R-24) is not decided by the synthetic scene: it needs the maintainers' E1
  numbers from a real level (the submit and present share of the game thread's frame). The bench
  gives the CPU side here: the render call and present take 0.81 ms (Mac) and 1.10 ms (box1) at
  1,460 draws, far from 6 ms.
- **G2** (R-20, Hi-Z occlusion) asks for world passes bound by vertices with many instances hidden,
  or overdraw above two fragments a pixel. The half-size test shows the world passes vertex-bound
  on box1; how many instances are hidden needs a real level (level_12) and an indoor one, with
  Enhanced on.
- **G4** (R-17) is R-16's report: the moby layer's CPU per between-frame. The base gives mobys
  0.357 ms (Mac) and 0.640 ms (box1) of CPU per picture at 300 mobys, and the `between x3`
  setting the same per picture; R-16 measures it again.
- Each firm package's own counter is in the tables above: `vk_cmds_recorded` and its kinds (R-03),
  `bench_heap_allocs` (R-04), `vk_ring_bytes` and the host-visible bytes (R-05),
  `vk_attachment_store_bytes` (R-06), `vk_fullscreen_copies` (R-07), `vk_barriers_all_commands`
  (R-08), the pipelines after the warm-up (R-10, R-11), the glslang runs (R-12), the textures made
  in the warm-up (R-13), the descriptor sets and writes with materials off (R-14),
  `vk_gpu_payload_bytes` (R-15).

### The maintainers' commands for real levels

The level is the editor's port export of it (`manifest.json` beside `placements.json`,
`editor/port.py`); with the release preset built:

```sh
python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_01 --reduced --sections --label level_01
python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_12 --reduced --sections --label level_12
# one setting by hand, as JSON lines
port/build/release/openrac_render_bench --level assets/port/rac1/pal/level_12 --enhanced on --frames 300
# a package's entry on and off on a real level
python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_12 --compare-opt NAME
```

On a real level the camera walks its bounds from one corner to the other, and the live mobys are
300 of the level's own classes, each walking a circle the seed places (the game's own mobys need
the game running, which the benchmark does not).

## R-03: no clock per draw, state and binds recorded when they change

**What changed** (`renderer/vulkan/vkgl_draw.cpp`, `vkgl_internal.h`, `vkgl_commands.cpp`):

- `vk-draw-clock`. Every draw read `Clock::now()` twice for `measures.draw_s`. Its only readers are
  the gauges `vk_draw_ms` and `vk_draw_us_each`, published only while `stats::enabled()`, so the
  clock is now read only then; `vk_draws` still counts every draw. The `OPENRAC_FRAME_COST` line
  does not show `draw_s` and is unchanged.
- `vk-state-skip`. Every draw that was not merged into an indirect batch recorded six dynamic-state
  commands (viewport, scissor, blend constants, stencil reference, compare and write masks) and
  bound its descriptor set, the material set and its push constant, its vertex buffers and its
  index buffer, whether or not the command buffer held them. `Context::recorded`, a
  `RecordedState`, remembers what was last recorded into the frame's main command buffer, and a
  draw records only the commands whose values differ (compared by their bytes, so the blend
  constants' -0 and +0 stay apart). Vulkan keeps that state for the life of a command buffer; the
  tracker is forgotten at every begin of rendering and every begin of the command buffers, which
  is more often than Vulkan needs, and a change of pipeline layout forgets the sets and the push
  constant. The pipeline was tracked already (`Context::bound_pipeline`).
- The `OPENRAC_GPU_MARKERS` read in the timed scopes (REPORT C4) already goes through
  `common/env.h` since P2-20, and is compiled only with `-DOPENRAC_GPU_MARKERS=ON`; nothing changed
  there.

**The counter**, per measured frame of `render_bench.py --synthetic --compare-opt vk-state-skip`
(the entry off, then on), 1920x1080, five runs a setting. The counts are the same on the Mac and on
box1, and in every run:

| Setting | `vk_cmds_recorded` | `vk_cmds_dynamic_state` | `vk_cmds_bind_descriptors` |
|---|---|---|---|
| base (walk, cold; also warm, static, between, half size) | 174 to 98 (-43.7%) | 60 to 6 (-90.0%) | 31 to 27 (-12.9%) |
| enhanced | 287 to 194 (-32.4%) | 96 to 24 (-75.0%) | 45 to 44 (-2.2%) |
| gpu-driven off | 17,554.6 to 3,293.1 (-81.2%) | 8,757.8 to 6 (-99.9%) | 4,378.9 to 1,475.6 (-66.3%) |
| materials off | 14,654 to 2,982 (-79.7%) | 8,760 to 6 (-99.9%) | 1,461 to 1,461 (0%) |

Draw commands, draw calls, triangles, render passes, barriers, submits, ring bytes, descriptor sets
and writes, and pipelines are the same with the entry on and off. With materials off every draw
has a descriptor set of its own (one per texture binding), so its binds stay: that is R-14's.
`bench_heap_allocs` is unchanged (the per-draw allocations are R-04's). `vk-draw-clock` changes no
counter.

**Milliseconds, as information** (the picture's CPU time, median of five runs, entry off to on):

| Setting | Mac | box1 |
|---|---|---|
| base | 0.81 to 0.82 | 1.09 to 1.06 |
| enhanced | 1.95 to 1.99 | 3.23 to 3.31 |
| gpu-driven off | 1.28 to 0.93 (-27%) | 1.69 to 1.17 (-31%) |
| materials off | 1.32 to 1.11 (-16%) | 1.64 to 1.28 (-22%) |

Where the GPU-driven batches already merge the draws (base, enhanced) the difference is in the
noise; on the per-draw paths a quarter of the picture's CPU time goes. `vk-draw-clock` on against
off is within the noise on both machines (Mac within 1%, box1 between -11% and +14% on pictures of
about 1.1 ms).

**Equivalence.** `render_equivalence_vulkan` compares both entries on and off in every scenario,
Enhanced off and on, byte for byte, under the validation layer: identical, no validation message,
on both machines, and again with `OPENRAC_GPU_DRIVEN_OFF=1` and with `OPENRAC_GPU_MATERIALS_OFF=1`
(run by hand, the per-draw paths where the tracker skips most). The tracker's risk is a wrong
viewport, scissor or stencil state in some draw; the image tests that exercise those passed
unchanged with the entries on and with `OPENRAC_RENDER_OPT_OFF=all`: `vulkan_uniform`,
`renderer_direct_vulkan` (2D, scissor-heavy), `sun_shadow_vulkan` (indexed viewports) and the
rest of the Vulkan set. The fixture gained no scenario: the per-draw modes above are its scene
with thousands of state changes a frame.

Measured with:

```sh
python3 port/tools/perf/render_bench.py --synthetic --compare-opt vk-state-skip
python3 port/tools/perf/render_bench.py --synthetic --compare-opt vk-draw-clock
OPENRAC_GL_API=vulkan OPENRAC_VULKAN_PIPELINE_CACHE=off OPENRAC_VULKAN_VALIDATION=1 \
  OPENRAC_GPU_DRIVEN_OFF=1 ./port/build/release/render_equivalence_test
```

On real levels the maintainers' command is
`python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_12 --compare-opt vk-state-skip`.

## R-04: an allocation-free draw

**What changed** (`renderer/vulkan/vkgl_pipelines.cpp`, `vkgl_draw.cpp`, `vkgl_internal.h`,
`vkgl_gl.cpp`), all behind `vk-draw-cache`:

- **The pipeline lookup.** `pipeline_for` turned its `PipelineKey` (376 bytes) into a
  `std::string` at every draw, one heap allocation, to look it up in an
  `unordered_map<std::string>`. `PipelineTable` keeps each pipeline once with two indexes: a flat
  table of slots found by a 64-bit hash of the key's bytes (FNV-1a over its 8-byte words, then
  SplitMix64's finaliser), which compares the whole key on a hit, so a collision can never return
  another key's pipeline; and the string index, which the reference path still uses. The key is
  zeroed whole before it is filled, so its padding is part of what is compared. A deleted
  program's pipelines leave both indexes (`erase_program`).
- **The temporaries.** The draw's list of sampled textures and `descriptors()`'s blocks, key,
  writes and infos were vectors made each time. They are now `Context::scratch`'s, cleared and
  filled again so their capacity stays. A program's sets of the frame are `Program::set_cache`:
  the keys' words in one array and a fixed table of 512 slots, compared word for word on a hash
  hit, cleared with the frame (its capacity kept) and holding at most 256 sets, as the map it
  stands beside did. The reference path is the code as it was, sharing the frame's reset and the
  uniform upload (`begin_program_sets`), `descriptor_key` and `write_set` with the cached one.
- The 8-bit index emulation, which no renderer reaches (REPORT C6), logs a warning the first
  time a draw takes it.

Not done as the brief planned, for the maintainers to note: there is **no state epoch**. A draw
still fills its pipeline key and its set's key, and finds them in the tables. Skipping that work
when no GL setter ran would also need a bump from every input the keys read that no setter
touches: a buffer's ring slice, a uniform upload, the frame's ring, `glDeleteBuffers` turning an
input into the default one, a texture specified again with another format, and the indexed
viewports (set in `vkgl.cpp`, outside R-04's paths). A missed bump draws with a wrong pipeline or
a stale set; the allocation the brief targets is gone without that risk. For the same reason the
per-program sampler list and the vertex-input description are not cached: neither allocated.

**The counters**, per measured frame of `render_bench.py --synthetic --compare-opt vk-draw-cache`
(the entry off, then on), 1920x1080, five runs a setting:

| Setting | `bench_heap_allocs`, Mac | per draw, Mac | `bench_heap_allocs`, box1 | per draw, box1 |
|---|---|---|---|---|
| base (walk, cold; also warm, static, between, half size) | 7,103 to 1,235 | 4.87 to 0.85 | 7,118 to 1,250 | 4.88 to 0.86 |
| enhanced | 16,917 to 2,272 | 5.77 to 0.77 | 16,957 to 2,312 | 5.78 to 0.79 |
| gpu-driven off | 7,102 to 1,235 | 4.87 to 0.85 | 7,117 to 1,250 | 4.88 to 0.86 |
| materials off | 12,787 to 33 | 8.76 to 0.02 | 12,802 to 48 | 8.77 to 0.03 |

On the individual path (materials off) the draw allocates nothing: what is left is about 30 a
frame outside the draws. With the native material path about 1,200 a frame are left, less than one
a draw; they are made outside R-04's files (`gpu_materials.inc`'s page of material records, filled
again each frame, and the GPU-driven batches of `gpu_draw.inc`), and PROGRESS.md lists them.

`vk_pipelines_created` (7 in the load and warm-up, 16 Enhanced, 0 in the measured frames),
`vk_descriptor_sets_allocated` (5 a frame, 13 Enhanced, 939 materials off) and
`vk_descriptor_writes` (615, 681, 3,755) are the same with the entry on and off on both machines:
the same keys give the same pipelines and sets, and no two keys were merged. Every other counter
is unchanged too.

**Time, as information.** `vk_draw_us_each` (the backend's CPU time a draw, median of 3 runs of 60
frames), off to on: Mac 0.344 to 0.258 microseconds (base), 0.490 to 0.324 (materials off); box1
0.610 to 0.485, 0.673 to 0.544. The picture's CPU time, median of five runs, off to on: Mac 0.87 to
0.76 ms (base), 2.03 to 1.79 (Enhanced), 1.15 to 0.93 (materials off); box1 1.29 to 1.18 ms
(GPU-driven off) and 1.41 to 1.21 (materials off).

**Equivalence.** `render_equivalence_vulkan` compares the entry on and off in every scenario,
Enhanced off and on: identical on both machines, with no validation message, and again by hand
with `OPENRAC_GPU_DRIVEN_OFF=1` and with `OPENRAC_GPU_MATERIALS_OFF=1`. `vulkan_uniform` gained the
cache's own test, written before the change and passing on the code before it: for a blend factor,
a depth function, a vertex attribute's format and a bound texture, states A and B draw different
pixels and, after A, B, A with no draw between, the draw is A's again and makes no new pipeline;
then a program whose pipeline was made before the others' is deleted and every state is drawn
again (the validation layer reports a destroyed pipeline bound: a first version of
`erase_program` built on `std::remove_if` destroyed the wrong pipelines, and this check is what
fails on it). `vulkan_uniform`, `vulkan_uniform_reference` (`OPENRAC_VK_UNIFORM_REUSE_OFF`) and
the rest of the Vulkan set pass with the entry on and with `OPENRAC_RENDER_OPT_OFF=all`.

Measured with:

```sh
python3 port/tools/perf/render_bench.py --synthetic --compare-opt vk-draw-cache
./port/build/release/openrac_render_bench --materials off --opt-off vk-draw-cache
```

On real levels the maintainers' command is
`python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_12 --compare-opt vk-draw-cache`.

## R-05: static buffers in device memory

**What changed.** A GL buffer filled by `glBufferData` with a static usage (`GL_STATIC_DRAW`,
`_READ`, `_COPY`) and of 64 KiB or more gets a copy of its own at its first draw
(`vkgl_resources.cpp`, `make_resident`). Before, it went through the frame's ring for three frames
and then into host-visible memory, which a discrete GPU without resizable BAR reads over the bus
at each draw. Where the device's largest device-local heap has a mapped memory type (unified
memory: Apple silicon, integrated GPUs; or resizable BAR), the copy is one mapped allocation
written in place: device memory already, nothing to copy. Elsewhere it is device-only memory,
filled by a copy from a staging buffer in the recording's `upload` command buffer, which runs
before `main` draws and is ordered before it by the barrier at the end of `upload`.

Once the copy exists, the buffer's CPU copy (its mirror) is let go. Every reader of its bytes calls
`buffer_bytes`, which reads them back first and keeps them from then on:

| Reader | Where | Served by |
|---|---|---|
| `glBufferSubData` (the rest of the buffer stays) | `vkgl_gl.cpp` | the read-back; the buffer then leaves the at-once path and takes the three-frame rule, as before |
| A per-instance attribute, copied into the ring each frame (`persistent = false`) | `resolve` | the read-back |
| 8-bit indices, widened to 16 bits on the CPU | `vkgl_draw.cpp` | the read-back |
| A uniform block shorter than its declaration, padded on the CPU (`block_buffer`) | `vkgl_pipelines.cpp` | `glBindBufferBase(GL_UNIFORM_BUFFER)` reads it back and keeps that buffer's mirror for good |
| The GPU-driven instances' bounds and payload (`gpu_make_batch`) | `gpu_draw.inc` | not needed: those buffers are drawn per instance and never get a copy of their own, so their mirror stays |

The read-back takes the bytes from mapped memory, or from the staging buffer while the copy it
feeds is not submitted yet, or else with a command buffer of its own: a barrier behind every
earlier transfer, a copy to a mapped buffer, a submit with its own fence and a wait. It leaves the
frame's command buffers, GPU-driven pages and descriptor sets as they are, so it may happen in the
middle of a draw. The backend has no `glGetBufferSubData`, `glMapBuffer` or `glCopyBufferSubData`.

Registry: `static-buffers-device-local` (on); `static-buffers-force-staging` (off) takes the staging
path on a mappable device too, wherever the device has device-only memory (MoltenVK has).

**The counters.** `openrac_render_bench --warmup 3 --frames 10` (1920x1080, the base setting), the
load, the three frames after it and the ten measured frames, master at `51188bc3` against R-05; the
same in all five runs on each machine:

| Counter | Mac before | Mac after | box1 before | box1 after |
|---|---|---|---|---|
| `vk_ring_bytes`, three frames after the upload | 73,402,848 | 57,821,184 | 73,402,848 | 57,821,184 |
| `vk_cpu_mirror_bytes` after those frames | 5,710,008 | 516,120 | 5,710,008 | 516,120 |
| `vk_buffer_static_bytes_host_visible` added in those three frames | 0 | 5,193,888 | 0 | 0 |
| the same added in the measured frames (the old three-frame rule) | 5,193,888 | 0 | 5,193,888 | 0 |
| `vk_ring_bytes`, `vk_submits`, `vk_barriers` per measured frame | 169,984, 1, 10 | the same | 169,984, 1, 10 | the same |

The ring takes 15,581,664 bytes fewer in the first three frames: three copies of the scene's
5,193,888 bytes of static vertices and indices. The mirror falls by the same 5,193,888 bytes; the
516,120 left are the dynamic and instance buffers. On box1 (no resizable BAR) the static buffers
are now in device-only memory: 0 host-visible bytes, against 5,193,888 before. On the Mac
(unified memory) the same bytes are host-visible before and after, as expected; they arrive
three frames earlier. The gain is RAM everywhere and bus traffic on a discrete GPU without
resizable BAR; nothing on Apple GPUs. With Enhanced on the mirror falls from 14,010,556 to
8,816,668 bytes and the ring by the same 15,581,664. Once the scene has settled the per-frame
counters are the same as before, so `render_bench.py --synthetic --compare-opt
static-buffers-force-staging` (and `static-buffers-device-local`) show +0.0 percent on every
counter of their tables; the table does not list the mirror and host-visible totals, which the
bench's summary line carries.

Milliseconds, information only (median of five runs, lowest to highest), the picture's CPU time,
against master at `51188bc3`: Mac 1.93 (1.58 to 2.13) before, 2.09 (1.70 to 2.43) after, with the
Mac under other load; box1 1.12 (1.11 to 1.13) before, 1.13 (1.12 to 1.28) after. Within the
spread; the `--compare-opt` runs on box1 agree within 3 percent but for one row (14 percent, its
own spread).

**Equivalence.** `render_equivalence_vulkan` draws both entries on and off in every scenario,
Enhanced off and on, the level re-upload included: identical frames (0 pixels differ) on both
machines. `vulkan_buffer` (new) draws a 128 KiB static vertex buffer and index buffer and reads
the picture back after the first frames, after `glBufferSubData` in the first frame and later,
after `glBufferData` again, through 8-bit indices and as a uniform block, under the reference
path, the change and the staged change; it checks that the change's first frame takes neither
buffer through the ring and keeps no mirror, and that the staged path puts nothing in host-visible
memory. Its pixel checks passed before the change. `viewer_vulkan` and the other Vulkan image
tests pass unchanged, with `OPENRAC_RENDER_OPT_OFF=all` too; on box1 the OpenGL tests ran and
pass.

Commands:

```sh
cd port && cmake --preset release && cmake --build --preset release
ctest --preset release -R "vulkan|viewer|renderer|render_" --output-on-failure
python3 tools/perf/render_bench.py --synthetic --compare-opt static-buffers-force-staging
./build/release/openrac_render_bench --warmup 3 --frames 10
./build/release/openrac_render_bench --warmup 3 --frames 10 --opt-off static-buffers-device-local
```

On real levels (the maintainers'): `openrac_render_bench --level assets/port/rac1/pal/level_12
--warmup 3 --frames 10`, with and without `--opt-off static-buffers-device-local`, and the summary
line's `warmup` counters.

## R-10: pipelines made ahead on a worker

**What changed** (`renderer/vulkan/vkgl_pipelines.cpp`, `vkgl_internal.h`, `vkgl_device.cpp`,
`pipeline_cache_file.*`, the new `cache_file.*`), behind `pipeline-prewarm`:

- **The manifest.** Every pipeline key the drawing thread makes is recorded with its program's
  identity: a hash of its SPIR-V stages and of whether it uses the material pages and dynamic
  uniforms (its layout), so the same shaders with the same defines are the same program in the
  next run whatever their GL names. At close the manifest is saved beside the pipeline cache
  (`<cache>.manifest`), in the envelope the cache uses, now one helper for both
  (`cache_file.h`: magic, length, FNV-1a hash, a temporary file and a rename). Its data says its
  format (`kManifestFormat`, which R-11 raises) and its key size, so another build's manifest is
  dropped whole. Each key carries the runs since one made or used it; a key unused for eight runs
  (a program no longer built) leaves the manifest, which holds at most 4,096 keys. Another build's
  manifest is removed with its cache.
- **The worker.** When a program links, the manifest's keys of its identity are queued for a
  worker thread (`PipelinePrewarm`, a `JoiningThread` the backend stops and joins first at close).
  It calls only `vkCreateGraphicsPipelines`, into the shared `VkPipelineCache` (internally
  synchronised), through `create_graphics_pipeline`, the creation split out of `pipeline_for`, so
  both threads make the same pipeline from the same key. A draw that needs a key the worker has
  made takes it into `Context::pipelines`; a key still queued is moved to the front and waited
  for, and one being made is waited for, so no key is made twice (a wait is never longer than the
  drawing thread's own compile would have been, plus the one job ahead of it); any other key is
  made as before. Nothing is skipped and no other pipeline stands in. A program's release takes
  its jobs back and waits for the one being made before its modules go.
- The trigger is the program's link, so the hooks are in `vkgl_gl.cpp` (`link_program`,
  `release_program`, one call each), and the tests wait for the worker through
  `vkgl::wait_for_pipeline_prewarm()` (`vkgl.h`): both outside the package's listed files, for the
  maintainers to note.

**The counter.** On the synthetic scene `vk_pipelines_created_after_warmup` is 0 before and after
in every setting: the warm-up of ten pictures already holds every first use. What the pre-warm
moves is the pipelines the drawing thread makes in the load and the warm-up, per run, of
`render_bench.py --synthetic --compare-opt pipeline-prewarm` (the entry off, then on; five runs):

| Setting | Mac | box1 |
|---|---|---|
| warm cache (the run drawn twice in one process; the second reported) | 4 to 0 | 4 to 0 |
| base, cold (a new process with no cache and no manifest) | 7 to 7 | 7 to 7 |
| enhanced, cold | 16 to 16 | 16 to 16 |

A first run on a new machine (cold) has nothing to make ahead, and says so. In the warm run the
four pipelines of the programs linked again are made on the worker. Every other counter is the
same with the entry on and off. `vulkan_prewarm` covers a new process: a first run draws three
states of a program and closes, and the second run's draws make none of the three.

**Time, as information** (the first picture after the upload, median of five runs, off to on):
warm cache Mac 14.5 to 13.7 ms, box1 6.7 to 6.7 ms; cold base Mac 22.4 to 22.5, box1 30.4 to
30.6; the upload itself is unchanged (Mac 142.6 to 142.3 ms warm). On the synthetic scene the
saving is small because the pipeline cache already makes the warm compiles cheap; a real level
with its tens of programs is the maintainers' measurement (below).

**Equivalence.** `render_equivalence_vulkan` compares the entry on and off in every scenario,
Enhanced off and on, byte for byte: identical on both machines, no validation message, and again
by hand with `OPENRAC_GPU_DRIVEN_OFF=1` and `OPENRAC_GPU_MATERIALS_OFF=1`. The pipelines are made
by the same function from the same keys. `vulkan_prewarm` (new) checks the second run, a run of
another program (nothing made ahead), the entry switched off (nothing made ahead) and a damaged
manifest (dropped, the draws make their own); it passed ten runs in a row on both machines.
`pipeline_cache_file` gained the manifest's checks (read back; another key size, cut short or
another magic dropped; a key of the wrong size not saved). Built with `-fsanitize=thread` on the
Mac, `vulkan_prewarm_test`, `vulkan_uniform_test`, `render_opt_test --vulkan` and
`pipeline_cache_file_test` ran with no ThreadSanitizer report.

Measured with:

```sh
python3 port/tools/perf/render_bench.py --synthetic --compare-opt pipeline-prewarm
ctest --preset release -R '^vulkan_prewarm$' --repeat until-fail:10
```

On real levels the maintainers' commands are two runs of the game on the same build (the first
writes the manifest), or
`python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_12 --compare-opt pipeline-prewarm`
(its warm-cache setting).

## R-13: a level's textures made at its load

**What changed** (`renderer/texture_pool.*`, `scene/level_renderer.cpp`, `tests/render_fixture.h`):

- `texture-prepare`. A texture of the pool was made by the first draw that used it
  (`TexturePool::gl_texture`: `glGenTextures`, `glTexImage2D`, `glGenerateMipmap`, the sampler
  parameters), so the first frame after a level change made every texture it saw, each an image, a
  staging copy and a blit chain in that frame's upload command buffer on Vulkan. That code is now
  `create_gl`, the one place a pool texture is made, and `TexturePool::prepare(handles)` runs it
  for every handle that has no GL texture yet. `LevelScene::upload` hands it every texture its
  materials draw with (colour and emission images, white where there is none) after its buffers,
  before it returns: while the level loads and no frame is shown. The uploads go in batches of at
  most 32 MiB of texels with a `glFinish` between two, and the load logs one verbose line
  (`level textures: 601 made at the load, 54.7 MiB uploaded, 72.9 MiB with mip chains, 2 batches`
  on the synthetic scene). Off, every texture is made at its first draw, as before.
- Not changed: textures made lazily by anything else (the HUD's, particles', uploaded and field
  textures) stay lazy; no placeholder and no per-frame budget (both would show). PR #43's 2x
  colours are still built at a texture's first draw with an edge-aware filter: they follow the
  frame's filter setting and each bind's wrap modes, which the load does not know.
- Lifetime: level textures were never evicted (`end_frame` drops only uploaded and field
  textures), so making them early changes when they appear, not how long they live. Memory: all of
  a level's material textures are resident from the load on, seen or not: 72.9 MiB with mip
  chains on the synthetic scene (600 textures; 54.7 MiB as uploaded).
- The fixture's `reupload` made the scene's textures again only in name: `LevelScene::release`
  keeps its texture handles, so a second upload into the same object appended 600 new handles and
  the materials drew the first ones. `SceneRenderer::reupload` now makes a new `LevelScene`, as the
  frontend's `use_level` does, so the scenario makes the level's textures again (1,201 images in its
  reload frame, before 1).

**The counter.** The benchmark publishes the counters once a present, so a frame's gauges also hold
whatever the load did since the last present; the summary's `load` and `warmup` counters are taken
by `vkgl::submission_counters()` around `SceneRenderer::open` and are exact. The warm-up is the
level's first ten frames. `render_bench.py --synthetic --compare-opt texture-prepare` (entry off,
then on), 1920x1080, five runs a setting, the same on the Mac and box1 and in every run:

| Setting | textures created in the first ten frames | texel bytes uploaded in them |
|---|---|---|
| base (walk, cold), warm cache, static camera, between x3, gpu-driven off, materials off, half size | 1,201 to 1 (the moby palette) | 58,949,632 to 1,638,400 (the palette, ten frames) |
| enhanced | 1,208 to 7 (the palette and six images of the Enhanced passes, made at their first frame) | 58,949,636 to 1,638,400 |

The level textures made inside the first ten frames go from 600 (1,200 images: each texture's
image, then its mip chain's) to 0, and their bytes from 57,311,236 to 0; the load makes them
instead (load counters: 3 images to 1,203). Every per-frame counter of the measured frames is the
same with the entry on and off (draws, triangles, commands, ring bytes, descriptor writes, heap
allocations).

On the `reupload` scenario (`openrac_render_bench --scenario reupload --warmup 0 --frames 40`,
the level uploaded again before frame 20), no frame after a load's first made a texture, entry on
or off: the synthetic camera sees all 600 materials in its first frame, so the whole cost is in that
frame. On a real level the camera finds materials as it moves, so with the entry off textures are
also made in later frames; with it on none are.

**Milliseconds, as information** (median of five runs, lowest to highest; entry off to on):

| Timing | Mac (M5 Max, MoltenVK) | box1 (GTX 1070) |
|---|---|---|
| the level's upload | 117.6 (116.1 to 121.0) to 206.1 (203.3 to 208.3) | 187.6 (186.1 to 194.9) to 303.2 (300.7 to 308.8) |
| its first picture | 76.4 (71.0 to 81.5) to 22.2 (22.0 to 22.9) | 133.1 (123.3 to 136.7) to 29.8 (28.8 to 30.4) |
| upload and first picture | 194.0 to 228.3 | 320.7 to 333.0 |
| the first picture after the reupload | 40.0 (39.7 to 43.2) to 13.9 (13.5 to 14.0) | 98.6 (89.5 to 103.2) to 6.8 (6.8 to 8.9) |

The first picture of a level loses 54 ms on the Mac and 103 ms on box1, and the load gains about
88 and 116 ms: the same work moved out of the frame loop, plus the two batches' `glFinish` waits.
The picture time of the measured frames is unchanged (within the noise on both machines).

**Equivalence.** `render_equivalence_vulkan` draws `texture-prepare` on and off in every scenario,
Enhanced off and on, the `reupload` one included (its first frames after the load are compared),
byte for byte under the validation layer: identical, no validation message, on the Mac and box1.
The whole release preset passes on both (the OpenGL tests, `viewer`, `viewer_es` and
`renderer_texture` among them, on box1; they skip on the Mac), and the headless preset on the Mac.
The change reaches the GL and GLES paths through the pool: `viewer`, `viewer_es` and
`renderer_texture` are the tests to run on a machine with a GL context.

Measured with:

```sh
python3 port/tools/perf/render_bench.py --synthetic --compare-opt texture-prepare
OPENRAC_GL_API=vulkan port/build/release/openrac_render_bench --scenario reupload --warmup 0 --frames 40
OPENRAC_GL_API=vulkan port/build/release/openrac_render_bench --scenario reupload --warmup 0 --frames 40 --opt-off texture-prepare
OPENRAC_LOG=verbose OPENRAC_GL_API=vulkan port/build/release/openrac_render_bench --frames 1 --warmup 0
```

On real levels the maintainers' commands are
`python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_01 --compare-opt texture-prepare`
(the warm-up rows) and the same for level_12.

## R-06: attachments a pass does not keep

**What changed.** The Vulkan backend carries out `glInvalidateFramebuffer`, for a framebuffer
object (colour 0, depth, stencil, depth-stencil) and for the default framebuffer (`GL_COLOR`,
`GL_DEPTH`, `GL_STENCIL`); before, it was not in the backend's table and the optional function was
null there. Dynamic rendering fixes a pass's store operations when the pass begins, before the
renderer says what it no longer needs, so under `attachment-discard` a pass's draws are recorded
into a secondary command buffer (`main` points at it while the pass is open) and the pass is begun
in the frame's command buffer when it ends, with its operations chosen then
(`vkgl_commands.cpp`, `begin_deferred` and `end_deferred`):

- an aspect let go since the pass's last draw or clear is stored `DONT_CARE`;
- an aspect let go outside its pass (after the pass that drew it ended) is loaded `DONT_CARE` by
  the next pass on it;
- a draw or a clear after the call makes the attachment live again in the same pass, and a copy,
  blit, upload or depth resolve into an image makes it whole, so new data is never dropped.

`renderer/gl.cpp` loads OpenGL ES's optional functions on the Vulkan backend too, so the
renderer's call at the end of the frame (`renderer.cpp`, the frame's depth and stencil) reaches the
backend. `renderer/gl.h` does not change. Each pass records one command more
(`vkCmdExecuteCommands`, counted as other).

**What each pass reads and keeps.** The passes the synthetic scene's frame runs, from the
backend's own choice of operations (1920x1080, MSAA 1); the depth image is `D32_SFLOAT_S8_UINT` on
the Mac and `D24_UNORM_S8_UINT` on box1, one image for depth and stencil:

| Pass | Attachment | Loaded from | Stored for |
|---|---|---|---|
| Enhanced off: the frame (clear, sky, world, effects, HUD) | colour | cleared | the copy to the screen, and the next frame when it keeps its colour |
| | depth, stencil | cleared | nobody: let go at the frame's end (renderer.cpp), now not stored |
| Enhanced on, 1: the frame's clear, background and sky, until the shadow atlas begins | colour | cleared | pass 3 |
| | depth, stencil | cleared | pass 3 loads them; the world's depth prepass clears them in that pass after the sky's draws |
| Enhanced on, 2: the shadow atlas (3072 by 3072) | colour (RGBA8) | loaded | stored, though no pass samples it (follow-up for the scene's lane) |
| | depth | cleared | the world's colour passes sample it |
| | stencil | loaded (not cleared), so the whole depth image is read in | stored with depth |
| Enhanced on, 3: the world's depth prepass, then its opaque colour | colour | pass 1 | pass 4 |
| | depth, stencil | pass 1 | GTAO's depth copy and early lighting read the depth; pass 4 tests against it |
| Enhanced on, 4: blended work, effects, HUD | colour | pass 3 | the copy to the screen |
| | depth, stencil | pass 3 | nobody: let go at the frame's end, now not stored |
| MSAA 4: the frame | colour (4 samples) | cleared | the resolve's copy after the pass (R-07 moves it into the pass) |
| | depth, stencil (4 samples) | cleared | nobody: now not stored |

GTAO's passes and early lighting are compute dispatches and copies, not render passes, so they
have no attachments here. The present copy reads the frame's colour (R-07). The one call the
renderer makes is the end of the frame's: the other dead attachments above are in the scene's
files (the shadow atlas, `scene/level_renderer.cpp`), which another lane owns, and
[PROGRESS.md](../restructure/PROGRESS.md) lists them as follow-ups with their bytes.

**The counters.** `openrac_render_bench --warmup 3 --frames 10`, the registry entry off and on,
per measured frame, the same in all five runs (the counts are the same on both machines; the bytes
follow each machine's depth format):

| Setting | Mac store bytes, off / on | box1 store bytes, off / on | load bytes (both, unchanged) | passes |
|---|---|---|---|---|
| Enhanced off | 24,883,200 / 8,294,400 | 16,588,800 / 8,294,400 | 0 | 1 |
| Enhanced on | 187,895,808 / 171,307,008 | 125,263,872 / 116,969,472 | Mac 163,012,608, box1 108,675,072 | 4 |
| MSAA 4 | 99,532,800 / 33,177,600 | 66,355,200 / 33,177,600 | 0 | 1 |
| Enhanced on, MSAA 4 | 577,732,608 / 511,377,408 | 357,507,072 / 324,329,472 | Mac 445,022,208, box1 274,563,072 | 6 |

The depth and stencil stores of the passes nothing reads later are gone: 66.7 percent of the
frame's stores on the Mac and 50 percent on box1 without Enhanced, 8.8 and 6.6 percent with it,
where the shadow atlas and the passes before the last keep theirs. The loads do not change on this
scene: every pass that loads reads what an earlier pass of the frame stored for it.
`vk_cmds_recorded` grows by one a pass (98 to 99 a frame; 194 to 198 with Enhanced). A desktop
GPU gains nothing from this; a tiled one (Apple, Adreno) writes that much less to memory, which
the maintainers confirm with a capture (below).

Milliseconds, information only (median of five runs, lowest to highest), the picture's CPU time
off / on: Mac 0.87 (0.82 to 0.89) / 0.84 (0.82 to 0.87), Enhanced 2.01 / 2.13; box1 1.13 (1.12 to
1.27) / 1.12 (1.11 to 1.16), Enhanced 3.16 / 3.16. No change beyond the noise from recording each
pass into a secondary buffer.

**Equivalence, and the stronger proof.** `render_equivalence_vulkan` draws `attachment-discard`
on and off in every scenario, Enhanced off and on, and then draws every scenario once more with
`vkgl::test_poison_discards(true)`: each attachment let go is cleared to garbage (colour 1.0 in
every channel, depth 0, stencil 0xA5) as soon as it is let go, and a pass that would load it clears
it to the same. Those frames are compared with `attachment-discard` off: a pass that read discarded
data would draw other pixels. All twenty comparisons are identical on the Mac and on box1, under
the validation layer. The Vulkan image tests (`renderer_*_vulkan`, `gtao_vulkan`,
`early_lighting_vulkan`, `enhanced_rendering_vulkan`, `sun_shadow_*`, `probe_gpu_vulkan`,
`viewer_vulkan`) pass with the entry on and with `OPENRAC_RENDER_OPT_OFF=all`. On box1 the OpenGL
tests ran too; R-06's one change in shared code (`renderer/gl.cpp`) loads nothing new on an OpenGL
context.

Commands:

```sh
cd port && cmake --preset release && cmake --build --preset release
ctest --preset release -R "vulkan|viewer|renderer|gtao|early_lighting|enhanced_rendering|shadow|render_" --output-on-failure
python3 tools/perf/render_bench.py --synthetic --compare-opt attachment-discard
./build/release/openrac_render_bench --warmup 3 --frames 10 --enhanced on --msaa 4 --opt-off attachment-discard
```

**On a tiled GPU (the maintainers').** The gain is bandwidth on Apple GPUs and Adreno, which the
counters model but cannot measure. With Xcode on the Mac: build the release preset, run
`openrac_render_bench --enhanced on --frames 300` (or the game on level_12) under Instruments' Metal
System Trace or an Xcode GPU frame capture (Product, Profile, Metal System Trace; or the Debug
Navigator's Capture GPU Workload on the running program), once as is and once with
`OPENRAC_RENDER_OPT_OFF=attachment-discard`, and compare the render encoders' store actions (the
depth attachment's `Don't Care` against `Store`) and the memory-bandwidth counters (bytes written
by the GPU per frame). On Android, AGI's frame profiler shows the same as each render pass's
store operations and the GPU's external write bandwidth.

## R-15: static instances resident on the GPU

**What changed** (`renderer/vulkan/gpu_draw.inc`, `vkgl.h`, `vkgl.cpp`, `vkgl_internal.h`,
`scene/level_renderer.cpp`):

- `gpu-resident-instances`. For every group of terrain, ties and shrubs and every view (the camera,
  its PVS pass, each shadow view), `gpu_make_batch` copied each instance's 120 bytes of data and 32
  of bounds into the page payload, and `gpu_record` copied the payload again into the mapped input
  page, every frame, though static instances never change after the upload. `build_groups` now
  registers each static group once (`vkgl::gpu_register_instances`): its candidates, in the
  culling's 38-word layout, are appended to one array, which the first batch that needs it copies
  into a device-local storage buffer through the ring. A batch of a registered, unchanged buffer
  (its GL version) writes only the view, 58 words, and a group record whose first word carries
  `kGpuResidentBit`; the cull shader reads such a group's candidates from a fourth storage binding.
  The moby layer, regrouped each frame, keeps the payload copy.
- Lifetime: `LevelScene::release` calls `gpu_release_instances`; the device copy is destroyed by the
  frame slot's deferred list, after the slot that last bound it has retired. One recording binds one
  resident copy; a batch that would need a newer one (a level uploaded in the middle of a recording)
  takes the payload path for that recording. Ordered compaction, unknown bounds staying visible, the
  UI bypass and the shadow views' own planes are unchanged: the shader reads the same words from
  another buffer.
- The instance vertex buffers stay (the CPU path and the per-instance shadow paths draw from them),
  and so does the AMD persistent-buffer workaround of `vkgl_draw.cpp`.

**The counter**, per measured frame of `render_bench.py --synthetic --compare-opt
gpu-resident-instances` (entry off, then on), 1920x1080, five runs a setting, the same on the Mac
and box1 and in every run:

| Setting | `vk_gpu_payload_bytes` |
|---|---|
| base (walk, cold; also warm, static, between x3, materials off, half size) | 2,839,984 to 407,376 (-85.7%) |
| enhanced (camera, PVS pass and four sun cascades) | 9,824,306.9 to 1,310,295.5 (-86.7%) |
| gpu-driven off | 0 to 0 |

What is left is the views' 58 words and the moby layer's copied instances. Draw calls, triangles,
`vk_gpu_instance_batches`, `vk_gpu_instance_candidates`, `vk_gpu_completed_draws`,
`vk_gpu_completed_visible_instances` and `vk_gpu_completed_triangles` are the same with the entry on
and off: the same set is drawn. `vk_ring_bytes` per frame is unchanged (169,984; Enhanced 181,108):
the payload goes into the mapped input pages, not the ring; the resident copy is staged through the
ring once, in the first frame that draws (about 0.6 MB on this scene). `vk_descriptor_writes` is one
more per page with the entry on or off (616 base, before 615): the fourth binding is always written,
the input page standing in when no resident words are read. The glslang runs counted in the load go
from 38 to 41 with the entry on (cold cache): registering asks whether GPU-driven drawing is
active, which builds the three culling programs at the load instead of at the first frame. The
scene has 4,000 static instances; a scene of 8,000, as the brief named, needs the bench's instance
count, which `openrac_render_bench` does not offer yet.

**Milliseconds, as information** (the picture's CPU time, median of five runs, entry off to on):

| Setting | Mac (M5 Max, MoltenVK) | box1 (GTX 1070) |
|---|---|---|
| base | 0.84 to 0.80 | 1.15 to 1.00 |
| enhanced | 2.03 to 1.80 | 3.22 to 3.22 |
| static camera | 0.84 to 0.78 | 1.15 to 0.99 |
| materials off | 1.12 to 1.07 | 1.30 to 1.17 |

**Equivalence.** `render_equivalence_vulkan` draws the entry on and off in every scenario, Enhanced
off and on, the `reupload` one included (the level released and uploaded again while two frames are
in flight, the lifetime this design must get right), byte for byte under the validation layer:
identical, no validation message, on the Mac and box1. `viewer_vulkan`'s fixtures (stable
compaction, PVS changes, descriptor rollover, CPU against the GPU reference of
`tests/gpu_draw_reference.h`) and `hero_shadow_vulkan`, `sun_shadow_vulkan`,
`early_lighting_vulkan` and `probe_gpu_vulkan`, which consume the same batches, pass with the entry
on and with `OPENRAC_RENDER_OPT_OFF=all`. The whole release preset passes on both machines (the
OpenGL tests on box1), the headless preset on the Mac. The change is Vulkan's only: no GL test is
reached.

Measured with:

```sh
python3 port/tools/perf/render_bench.py --synthetic --compare-opt gpu-resident-instances
OPENRAC_GL_API=vulkan OPENRAC_VULKAN_PIPELINE_CACHE=off OPENRAC_VULKAN_VALIDATION=1 ./port/build/release/render_equivalence_test
```

On real levels the maintainers' command is
`python3 port/tools/perf/render_bench.py --level assets/port/rac1/pal/level_12 --compare-opt gpu-resident-instances`
(level_12's 7,468 instances: the research's 1.1 MB a view).

## R-18: a static shadow cache, shipped off

**What changed** (`scene/level_renderer.*`, `enhanced/viewer/sun_shadow.h`,
`tests/render_fixture.h`, `enhanced/tests/sun_shadow_test.cpp`):

- `shadow-static-cache`. With Enhanced graphics the shadow atlas (3072 by 3072) is cleared and every
  caster is drawn into the four sun cascades and the hero map each frame. With the entry on,
  `LevelScene` keeps a second depth target in the atlas's layout. For each cascade a
  `StaticShadowKey` holds what its static casters (terrain, ties, shrubs) depend on: the snapped
  view-projection, the caster volume (the receiver footprint, which culls the casters), whether the
  hero's casters are the hero map's, and a generation bumped by an upload and by `set_instances` of
  a static layer. A cascade whose key changed has its rectangle of the cache cleared and its static
  casters drawn again, with the atlas's own shaders, matrices and indexed viewports; then the atlas
  starts as an unscaled depth copy of the cache (`glBlitFramebuffer`, which the Vulkan backend
  carries out as `vkCmdCopyImage` of the depth aspect: the GL table gains nothing) and only the
  mobys and the hero map are drawn over it. The reversed depth test keeps the nearest of the cached
  and the new fragments, the same as drawing all of them in one pass, cutouts included (the same
  discards).
- Not done: staggering cascade updates over frames and reusing a cascade whose key changed (both
  show stale shadows), and caster culling against the camera's visible region (the cache's content
  must not depend on the camera).
- Found while doing it: the static caster set does not depend on the snapped matrix alone. The
  receiver culling (`sun_cascades`'s caster volume, `OPENRAC_SUN_RECEIVER_CULL_OFF` turns it off)
  follows the unsnapped camera, so any camera movement changes every cascade's key:
  `sun_shadow_test` shows a camera moved 0.0001 units keeping two of four snapped matrices and none
  of the four keys.
- New gauges, published by the atlas each frame with the entry on or off: `vk_shadow_draws` and
  `vk_shadow_triangles` (the atlas's draws and triangles) and `shadow_static_rerenders` (cascades
  whose static casters were drawn: 4 without the cache).

**The counters and the GPU time**, Enhanced on, 1920x1080, 60 measured frames after 10, five runs a
setting, the entry switched on in a measurement build (`openrac_render_bench` can only switch an
entry off, so `render_bench.py --compare-opt` cannot show a default-off entry on):

| Scenario | `vk_shadow_draws` | `vk_shadow_triangles` | `shadow_static_rerenders` | shadow atlas GPU ms, Mac | box1 |
|---|---|---|---|---|---|
| static camera, mobys moving (`static-mobys`), off | 755 | 3,976,950 | 4 | 0.131 (0.130 to 0.132) | 0.236 (0.235 to 0.236) |
| the same, on (every cascade a hit) | 184 | 432,000 | 0 | 0.237 (0.236 to 0.237) | 2.259 (2.259 to 2.259) |
| walk, off | 844 | 4,440,140 | 4 | 0.159 (0.158 to 0.159) | 0.316 (0.315 to 0.318) |
| walk, on (every cascade a miss) | 844 | 4,440,140 | 4 | 0.399 (0.398 to 0.401) | 2.433 (2.424 to 2.552) |
| `creep` (a hundredth of a unit a tick), on | 755 to 756 | | 4 | | |

The draws and triangles fall by 76 and 89 percent on a hit, the same on both machines, but the
copy of the whole depth image (3072 by 3072: 75.5 MB each way at the Mac's 8-byte depth, 37.7 MB at
box1's 4-byte depth and stencil) costs more than the draws: the atlas's GPU time grows 1.8 times on
the Mac and 9.6 times on box1, where copying the NVIDIA depth-stencil image is slow. With a moving
camera every cascade misses and the copy is added to the full redraw. The picture's CPU time
(information) on the static camera falls from 1.50 to 1.35 ms on the Mac and grows from 3.44 to
5.03 ms on box1 (its frame waits for the GPU). So the entry ships off, as the brief requires when
the GPU time is not lower. What could make a cache pay is a cheaper composition (copying only the
four cascade rectangles, or a depth pass that samples the cache) and a cache key without the
camera-dependent caster volume (drawing the cached static layer against the snapped square map,
which changes the atlas's content outside the receiver footprint and so needs the maintainers'
acceptance): follow-ups in PROGRESS.md.

**Equivalence.** `render_equivalence_vulkan` compares the entry on against off in every scenario,
the new `creep` one included (cascades that stay or move by a texel), static camera with moving
mobys, the reupload, Enhanced off and on, byte for byte under the validation layer: identical, no
validation message, on the Mac and box1. The frames compared are the final pictures, which sample
every cascade; reading the atlas back in the test is not done (the test program is outside R-18's
paths). `sun_shadow_vulkan`, `sun_shadow_filter_reference_vulkan`, `hero_shadow_vulkan` and the
whole release preset pass on both machines (on box1 one GLES test, `renderer_shader_es`, failed once
in a full run and passed alone twice, beside NVIDIA Xid faults; see R-05's note), the headless preset
on the Mac.

Measured with (the entry's default switched to on in `render_opt.cpp` for the "on" rows):

```sh
OPENRAC_GL_API=vulkan port/build/release/openrac_render_bench --enhanced on --scenario static-mobys --frames 60 --warmup 10
OPENRAC_GL_API=vulkan port/build/release/openrac_render_bench --enhanced on --scenario walk --frames 60 --warmup 10
# the "off" rows: the same with --opt-off shadow-static-cache
```
## R-07: one copy to the screen

**What changed.** The frontend's `present()` (`game/common/frontend.cpp`, not edited) clears the
window black and blits the frame's picture into a box of it; the Vulkan backend then copied the
window, an image of its own, into the swapchain image, flipped. Two full-screen reads and writes
of the picture each frame. Under `present-single-copy` the blit into the default framebuffer waits
(`PendingPresent`, `vkgl_gl.cpp`'s `hold_present_copy`) when the pass before it into that
framebuffer only cleared its colour and holds nothing yet: that pass is dropped unrecorded and its
clear colour kept. `present_to_screen` then clears the swapchain image to that colour and blits the
picture into it with the box turned upside down (`compose_into`). The composition is exact when
the second copy was 1:1 and NEAREST and both images hold the same numbers, so it is used when the
swapchain is the window's size and its format is `B8G8R8A8_UNORM` or `R8G8B8A8_UNORM` (the
window's is `R8G8B8A8_UNORM`).

Everything else makes the waiting copy first, as `glBlitFramebuffer` would have recorded it (a
clear, then the blit; `make_present_copy`), and carries on as before:

| What comes after the blit | Where it is caught |
|---|---|
| A draw or a clear into the window (ImGui, the debug overlay, the touch overlay, the FPS counter's blit) | the window's image barrier as its pass begins (`barrier()` in `vkgl_commands.cpp`) |
| `glReadPixels` of the window, a copy or blit from it or into it | the same barrier, to a transfer layout |
| A draw, a clear, an upload or a blit into the picture | the picture's barrier to any layout but a read |
| A frame that ends with no screen shown (no window, a minimised one, a lost surface) | `advance()`, as the next frame begins |
| A screen of another size or format | `present_to_screen`, before its copy |

The waiting copy needs a pass that can be dropped, which only `attachment-discard`'s deferred
passes are (R-06): with that entry off, the copy never waits. The picture's image is held by the
waiting copy, so a picture made again before the present stays alive until then.

**The counters.** `vk_fullscreen_copies` counts the copy to the screen, which exists only where
the window has a swapchain: none of the headless tests or the benchmark's hidden window has one,
so a normal frame's 2 to 1 (the box's blit, when it fills the window, and the window's copy, now
one blit) is the maintainers' to see, and with an overlay drawn after the blit the frame stays at 2
(the waiting copy is made first). On the synthetic scene's benchmark every counter is unchanged,
on and off (`render_bench.py --synthetic --compare-opt present-single-copy`, Mac and box1): it
presents nothing to a screen. `vulkan_present` prints the copies of one frame, on and off: 0 and
0 on both machines, for the same reason. The picture's CPU time on that bench, as information:
Mac within 1.5 percent either way, box1 within the spread of its runs. The bytes a frame no longer copies are the window's: 8,294,400 at
1080p (one read and one write of an RGBA8 image).

**Equivalence.** The headless default framebuffer has no swapchain and `glReadPixels` reads it:
`renderer_direct_vulkan`, `imgui_gl_vulkan` and `touch_overlay_vulkan`, which draw into it after
blits, pass unchanged. `vulkan_present` (new) has the backend draw the copy that waits into two
`B8G8R8A8` stand-ins of the screen, the single way and the two-step way, and compares them byte for
byte: a picture of four flat colours scaled into an inset box, flipped with LINEAR and not flipped
with NEAREST; then an overlay drawn into the window after the blit makes the copy at once, and the
window reads back as it does with the entry off. `render_equivalence_vulkan` compares the entry on
and off in every scenario: identical on the Mac and on box1 (its scenarios present nothing to a
screen, so this shows that the entry changes nothing else).

The real swapchain path is covered only by this stand-in and by a maintainer's look at the window:
the letterbox bars black, the picture the right way up, the overlays (F3, the pause menu, the touch
controls) drawn over it, a window resized and moved between displays of other scales, and
fullscreen.

**Not done: the MSAA resolve inside its pass** (the brief's part 2, `msaa-resolve-in-pass`). It is
left for a package of its own: [PROGRESS.md](../restructure/PROGRESS.md)'s follow-ups say what it
needs.

## R-08: barriers that name their stages

**What changed.** Five barriers waited on and for every stage (`ALL_COMMANDS`), and the frame's
one submit waited for the swapchain image at `ALL_COMMANDS`, so the uploads, the culling dispatches
and all of the frame's rendering waited for the presentation engine, though only the copy to the
screen needs its image. Under `vk-tight-sync` each barrier names the stages and accesses it orders,
and the copy to the screen goes into a command buffer of its own (`present_cmd`), submitted as a
second batch of the same `vkQueueSubmit`; that batch alone waits for the acquire, at the transfer
stage where it first touches the image. One submit a frame still: a second `vkQueueSubmit` was not
needed, so its cost on MoltenVK does not arise. The barriers stay core Vulkan 1.0
(`vkCmdPipelineBarrier`): the narrower masks need nothing of synchronization2, which the device
does not enable, so no device needs the old path for want of a feature.

**Every barrier** (`vk-tight-sync` on; off, each is `ALL_COMMANDS` to `ALL_COMMANDS` with memory
read and write, as before, but the mip chain's, which was transfer to `ALL_COMMANDS`):

| Where | Source stages and accesses | Destination stages and accesses | Resource | What it orders |
|---|---|---|---|---|
| `barrier()` out of and into a layout (`vkgl_commands.cpp`, `scope_of`), every image transition | the old layout's use: `UNDEFINED` top of pipe, nothing; `TRANSFER_SRC` transfer read; `TRANSFER_DST` transfer write; `SHADER_READ_ONLY` the vertex, fragment and compute shaders' reads; `COLOR_ATTACHMENT` colour output read and write; `DEPTH_STENCIL_ATTACHMENT` early and late tests and colour output, depth read and write and colour write | the new layout's use, the same table | one image, all its levels | the earlier use of the image (a copy, a sampling, a pass's loads, stores and resolves) before its next one, and the layout change between them; a pass into the same layout again still waits for the last pass's writes |
| End of `upload` (`submit`) | transfer and compute shader; transfer write, shader write | draw indirect, vertex input, vertex, fragment and compute shaders, transfer; indirect, index, vertex attribute, uniform and shader reads, shader write, transfer read and write | memory | `upload`'s copies (texture uploads, mip chains, R-05's staging copies) and the culling's dispatches before `main`'s indirect draws, vertex and index fetches, uniform and texel reads, compute and copies |
| `swapchain_barrier`, `UNDEFINED` to `TRANSFER_DST` (`vkgl_present.cpp`) | transfer; nothing | transfer; transfer write | the swapchain image | the acquire (waited for at the transfer stage) before the clear or blit into the image |
| `swapchain_barrier`, `TRANSFER_DST` to itself | transfer; transfer write | transfer; transfer write | the swapchain image (or a stand-in) | R-07's clear before the blit that overlaps it |
| `swapchain_barrier`, `TRANSFER_DST` to `PRESENT_SRC` | transfer; transfer write | bottom of pipe; nothing | the swapchain image | the copy before the present, which the batch's signal semaphore orders for the presentation engine |
| `generate_mipmaps`' last barrier (`vkgl_resources.cpp`) | transfer; transfer read and write | transfer; transfer read and write | the levels above the last | the chain's blits before the `barrier()` that follows at once and moves every level to be sampled, out of `TRANSFER_DST` at the transfer stage |
| `glMemoryBarrier` (`vkgl_pipelines.cpp`) | vertex, fragment and compute shaders; shader write | the same shaders, draw indirect, vertex input, transfer; shader and uniform reads, shader write, indirect, vertex attribute, index and transfer reads | memory | compute's writes before whatever reads them next: a shader, a draw's buffers, a copy into an image |

The other barriers were narrow already and are unchanged: the GPU-driven culling's
(`gpu_draw.inc`), `copy_buffer_to_texture`'s and R-05's read-back. `grep ALL_COMMANDS
port/renderer/vulkan/*.cpp` finds the reference paths only (`vk-tight-sync` off) and one
case of `scope_of` that no layout the backend uses reaches.

**The counters.** `openrac_render_bench --warmup 3 --frames 10` (1920x1080), per measured frame, off
/ on:

| Setting | `vk_barriers` | `vk_barriers_all_commands` | `vk_submits` | `vk_cmds_recorded` | picture CPU ms (information) |
|---|---|---|---|---|---|
| Enhanced off, before (master) | 10 | 5 | 1 | 99 | 0.77 |
| Enhanced off, R-08 | 10 | 0 | 1 | 99 | 0.77 |
| Enhanced off, R-08 with the entry off | 10 | 5 | 1 | 99 | 0.76 |
| Enhanced on, before | 27 | 22 | 1 | 198 | 1.77 |
| Enhanced on, R-08 | 27 | 0 | 1 | 198 | 1.78 |
| Enhanced on, R-08 with the entry off | 27 | 22 | 1 | 198 | 1.77 |

On the Mac; the counts are the same on box1 (R-02's baseline gives box1 the same 5 and 22, and
the barriers do not depend on the device), where the benchmark could not run once the logged-in
session it draws in had ended. `render_bench.py --synthetic --compare-opt vk-tight-sync` on the
Mac: `vk_barriers_all_commands` -100 percent in every setting, every other counter +0.0 percent,
the picture's CPU time within 2.3 percent either way.

`vk_submits` stays 1 a frame: the copy to the screen, where there is one, is a second batch of the
same submit. The real gain, the GPU no longer idle at the start of a frame waiting for the
presentation engine and up to a frame less latency in the vsync modes, cannot be counted headless.

The maintainers measure it: with RenderDoc on box1 (or Xcode's Metal System Trace on the Mac),
capture a few frames of the game on level_12 at vsync on, once as is and once with
`OPENRAC_RENDER_OPT_OFF=vk-tight-sync`, and read the time between the queue's wait on the acquire
semaphore and the first dispatch or draw of the frame (RenderDoc's Timeline and its event list;
in Xcode, the gap before the first compute encoder of the frame's command buffer). With the entry
on, the frame's work starts before the acquire completes and only the last blit waits.

**Equivalence.** A synchronisation change is not proven by images alone: (a) the Vulkan tests ran
with the validation layer and its synchronisation validation (`VK_LAYER_ENABLES=
VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT`) on the Mac, with no `SYNC-HAZARD`
and no validation error reported; box1 has no validation layer installed (`vulkaninfo` lists
none), so there the tests ran without it and the table is the review, and (b) `render_equivalence_vulkan` draws the entry on and off in every
scenario: identical frames. The table above is the review that a reader repeats line by line.

## R-19: the zero-coverage cull, measured and not landed

The CPU reference of the test and its test landed; the cull in the GPU culling did not, because
its frames are not byte-identical on MoltenVK. Its code (the cull shader's test, the view's matrix
and viewport, the camera and shadow views filling them in, a registry entry
`cull-zero-coverage`) is kept as a patch outside the repository for the maintainers
(`~/RACSwarm/api-agents/restructure/R-19-zero-coverage-gpu.patch`, three commits on top of
`ca9bf0d5`); the numbers below are that patch's.

**What landed** (`renderer/gpu_draw.h`, `tests/cull_coverage_test.cpp`, `cmake/Render.cmake`):

- The argument. An instance's triangles lie inside its bounds. With all eight corners in front of
  the eye (clip w above 1e-6) the projection keeps them inside the convex hull of the projected
  corners, so inside their rectangle in the view's pixels. With one sample a pixel the samples are
  the pixel centres (k + 0.5); widened by 1/8 pixel for the vertex snapping (Vulkan guarantees at
  least a sixteenth) and the projection's float error, a rectangle that holds no centre on one axis
  lights nothing, so dropping the instance changes no pixel. Bounds reaching behind the eye and
  multisampled views (samples may sit anywhere in a pixel) are kept; the GPU-driven path draws only
  triangle lists (`gpu_submit_draw`), and the UI bypass and the transparent order would be
  unchanged (the test comes after the PVS and plane tests; the ordered compaction drops without
  reordering).
- `covers_a_sample` in `gpu_draw.h` is the CPU reference, and `GpuCullView` carries what it needs
  (`coverage`, `clip`, `viewport`, `samples`; nothing fills them in yet, and the payload is
  unchanged).
- `cull_coverage` (new, every preset, no GPU): of 20,000 random bounds of a hundredth to two pixels
  over a 1080p picture, 3,442 are dropped, and a software rasteriser (pixel centres, edges
  included) lights no centre with any of 6 random triangles inside each.

**What the patch does**: the cull shader's `coversSample` makes the reference's operations in the
same order after the PVS and plane tests; a view's payload grows from 58 to 79 words (the clip
matrix, the viewport, a flag); the camera's view gets the frame's view-projection and the target's
size (none on a multisampled target), each shadow map its matrix and its atlas rectangle or own
target; vkgl leaves the test out when the bound target has more than one sample.

**The counters** (per frame, mean of 60 measured frames of the walk at 1080p, the same on the Mac
and box1 and in every run: the decisions are the compute shader's, the same on both devices):

| Setting | `vk_gpu_completed_visible_instances` | `vk_gpu_completed_triangles` | `vk_gpu_completed_draws` |
|---|---|---|---|
| Enhanced off, MSAA 1 | 6,184.6 to 4,649.7 (-24.8%) | 782,988 to 576,074 (-26.4%) | 1,459.7 to 1,426.6 |
| Enhanced on (camera and four cascades), MSAA 1 | 9,274.5 to 7,002.8 (-24.5%) | 2,002,775 to 1,610,382 (-19.6%) | 2,378.2 to 2,324.4 |
| MSAA 4, Enhanced off and on | unchanged | unchanged | unchanged |

That is the fixture's mix (a fifth of its static instances sub-pixel at the walk's end), not a
level's: on real levels expect a few percent of instances, more in the far cascades. Picture CPU
time (information, median of 60 frames): Mac 0.679 to 0.691 ms, box1 0.970 to 0.985 ms (base);
Enhanced Mac 1.556 to 1.546, box1 3.774 to 3.608.

**Equivalence: exact on box1, not on the Mac.** With the patch, `render_equivalence_vulkan`
compares the entry on against off in all twelve scenarios under the validation layer. On box1 (NVIDIA) every frame is
byte-identical. On the Mac (MoltenVK) most scenarios differ by 1 to 10 pixels of 57,600 (worst
channel 4 base, up to 27 with Enhanced), the same with the cull applied to the static camera layers
only, and the same at a margin of 3/8 pixel; at 1/2 pixel nothing differs, because at that margin
every rectangle holds a centre and nothing is culled. The CPU reference finds no culled bounds with
a lit centre, and the two devices cull the same instances, so on Apple's GPU fragments are made at
positions the argument does not allow (a rasteriser whose effective sample positions are about half
a pixel from the centres the Vulkan specification gives). The brief counts such a failure as a bug
in the argument, not a tolerance to widen. A default-off entry would still fail
`render_equivalence_vulkan` on every Mac (the test switches each entry on), so the GPU part is not
landed until the MoltenVK behaviour is understood (follow-up in PROGRESS.md).

Measured with:

```sh
./port/build/release/cull_coverage_test
OPENRAC_GL_API=vulkan OPENRAC_VULKAN_PIPELINE_CACHE=off OPENRAC_VULKAN_VALIDATION=1 ./port/build/release/render_equivalence_test
# the counters, with the patch applied, per setting
OPENRAC_GL_API=vulkan port/build/release/openrac_render_bench --frames 60 --warmup 10 --camera walk --msaa 1 [--enhanced on] [--opt-off cull-zero-coverage]
```

On real levels (the maintainers', with the patch applied):
`port/build/release/openrac_render_bench --level assets/port/rac1/pal/level_12 --enhanced on --frames 300`,
with and without `--opt-off cull-zero-coverage`, reading the `vk_gpu_completed_*` gauges.
