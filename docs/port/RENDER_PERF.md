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
