# Diagnostics recordings

A diagnostics recording measures a short stretch of play. It produces a small report
that says where each frame's time goes. Attach it to a bug report or paste it into a
chat. In the code it is called a *performance capture*.

It works the same on desktop and Android. Nothing runs until you start one.

## How to record

| Where | How |
|---|---|
| Android | Drawer > Developer tools: pick a **Recording length**, then **Record diagnostics**. Recording starts 3 s after the drawer closes. Opening the drawer stops it. |
| Desktop | Developer menu > **Record diagnostics**, or **Ctrl+F3** to start and stop. |
| Script | `OPENRAC_PERF_CAPTURE=FRAME:SECONDS[:FILE]` starts one at that frame. Combine it with `OPENRAC_PRESS` and `--frames` to get repeatable runs. |

While it records, a red `REC` line shows at the top and the debug panel is hidden.
Paused frames and movies are not counted.

When it finishes, the report is **copied to the clipboard** and saved as
`perf_<game>_<date>-<time>.zip`, which holds the report (`.md`) and every frame (`.jsonl`).
Send the whole zip.

- Android: the **Diagnostics recordings** folder in the Files app.
- Desktop: `perf/` next to the game.

**Tips:** keep it short and repeatable. Stay in one place in one level, keep the same
settings and record for 30 s. Record the same scene before and after a change.

### Simulating a slower device

**Simulate slow CPU** (2x, 4x, 6x), in the same menus, makes the game's threads that many
times slower. The GPU is not slowed. It gives a rough idea, not an exact match for a real
weak phone. Works on Linux, Android and macOS.

## What the report contains

- **Setup**: build, device, CPU and its clock speeds, thermal state (Android), GPU and
  driver, window size and settings.
- **Verdict**: whether frames hit their target and, if not, whether the CPU or the GPU is
  the bottleneck. It also says how much slower a CPU and GPU could be and still run at
  60 fps.
- **Frame times**: average, median, 90th and 99th percentile and worst, for the frame, the
  game code and the GPU. It also shows the *work* of each thread, without time spent
  waiting. On a weaker device only the work counts.
- **Sections**: every timed part of the frame, with CPU time, GPU time, draw calls and
  triangles.
- **Counters and gauges**: draws, uploads, binds and renderer stats.
- **Timeline**: one row per second. A frame rate that drops while the work stays the same
  means the device is overheating.
- **Worst frames**: the 8 slowest frames and what they spent time on.
- **Hot functions** (Linux and Android): the 30 functions each thread spends the most time
  in, found by sampling.

## What is measured, and where

| | Desktop | Android |
|---|---|---|
| CPU time per section | yes | yes |
| GPU time (OpenGL) | yes | Adreno, most Mali |
| GPU time (Vulkan) | yes | yes |
| Counters | yes | yes |
| CPU clock speeds | Linux | if the system allows it |
| Thermal state | no | Android 11+ |

Measuring costs well under 1 ms per frame. On phone GPUs (Adreno, Mali) the per-pass GPU
times are a little high. The whole-frame times and the verdict are accurate.

## Going deeper

The report tells you *which* part is slow. To look inside it:

- **Android GPU Inspector**, **Perfetto**, **Snapdragon Profiler** (Adreno),
  **Arm Performance Studio** (Mali)
- **RenderDoc** for a single frame. A build with `OPENRAC_GPU_MARKERS=ON` labels the passes
  ([DEBUGGING.md](DEBUGGING.md)).

## Code

- [common/perf_capture.h](../../port/common/perf_capture.h): builds the report (tested by
  `perf_capture`).
- [game/common/debug/capture.cpp](../../port/game/common/debug/capture.cpp): starting,
  saving and copying.
- [common/sampler.h](../../port/common/sampler.h): hot functions.
- [common/slow_cpu.h](../../port/common/slow_cpu.h): slow CPU simulation.
