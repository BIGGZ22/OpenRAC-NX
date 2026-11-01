# 480p review, blocking diagnostics and overlay verification

## Input and findings

Reviewed contact sheets extracted from all five attached 18:28--18:30 gameplay
clips in build/deko3d-test/480p-review/. They show the cutscene, Metropolis
landing and the camera rotation. Nintendo's recordings do not include the
Status Monitor overlay; their encoded frame rate is not the game's FPS.

openrac-rac1-pal (29).log starts at 18:19:29 and reports an internal frame of
854x480 for a 1280x719 picture (the swapchain is 1280x720). It contains no passive
CPU/GPU timing samples, no frame-cost report and no new vertex-cache startup
message. It therefore appears to predate the CPU-cache candidate built at
18:30. Do not treat this run as a test of that candidate or the release library.
The screenshot/recording being 720p does not contradict internal 480p rendering.

The selftests pass. The supplied driver log has the compute-capable queue
warning, not a new fatal queue/method error. No quantitative cause for the
sustained camera-rotation slowdown can be established from these logs alone.
Smooth audio while animation falls behind is consistent with independent audio
playback and slow game frames; this pass does not change audio synchronization.

## Changes

1. Default OPENRAC_DK_MIP_AUDIT from 6 to 0. Audits copy a mip level to CPU-readable
   memory and call flush_and_wait(). Explicit positive values still enable it.
2. Default OPENRAC_DK_DRAW_TRACE from 4 to 0 (its old comment incorrectly said 8).
   Each selected large skinned draw reads the joint palette back, waits for the
   GPU and performs CPU geometry inspection. The submitted log contains these
   traces during this run. They are useful diagnostics, not ordinary play work.
   These bounded diagnostics can explain some isolated hitches, not all drops.
3. Moved the existing shared-memory field publisher into overlay_protocol.h so
   tests exercise the same code as the Switch. Publish data before the active
   heartbeat with a release fence; continue to leave overlay-owned control
   fields and performance counters untouched. Render size and output viewport
   remain distinct. Actual swap intervals, not the selected FPS cap, drive FPS.
4. overlay-status.log now records applet type, native deko3d compatibility mode,
   published dimensions on changes, and a first valid integer FPS sample.
   Existing connection/map/reservation error reporting remains. Telemetry needs
   no env switch. CPU/GPU utilization is measured externally by Status Monitor;
   the game does not manufacture those utilization numbers.
5. Retained the CPU vertex-input cache from the preceding pass.

## Protocol evidence

Read upstream source snapshots cloned into the review directory:

- https://github.com/masagrator/SaltyNX/blob/master/saltysd_core/source/NX-FPS-Common.hpp
- https://github.com/masagrator/SaltyNX/blob/master/saltysd_proc/source/service.c
- https://github.com/masagrator/Status-Monitor-Overlay/blob/master/source/Utils.hpp
- https://github.com/masagrator/Status-Monitor-Overlay/blob/master/source/modes/Full.hpp
- https://github.com/masagrator/Status-Monitor-Deux/blob/main/source/Utils.cpp

Both readers use the active heartbeat and 0xffff resolution-request sentinel.
Both look for an application process, so applet launches can prevent discovery.
SaltyNX wire IDs are 1=NVN, 2=EGL, 3=Vulkan; there is no deko3d ID. We retain 2
as a non-NVN compatibility channel for direct FPS/resolution publication. This
does not claim an EGL renderer or emulate NVN performance counters. It would be
incorrect to relabel it NVN merely to display counters. This pass has not proven
that the user's sysmodule connects: overlay-status.log was not supplied.

## Tests and artifacts

switch_overlay_stats_test passes, including 60-to-20 FPS timing, reader heartbeat,
resolution sentinel acknowledgement, internal/output size distinction, size/API
changes, dimension bounds, zeroed spare entries, and preserved control fields.
Real dkgl renderer mock passes with zero failures, including cache invalidation.
No GPU readback pixel correctness is claimed from the mocked GPU.

Both Switch targets succeeded with inner exit code 0. Copied NRO/ELF hashes match
their build outputs. Patched and stock packs match staged RomFS files. Manifests
in both output directories are updated, hardware_tested=false.

| Build | ELF build ID | NRO SHA256 |
| --- | --- | --- |
| Diagnostic | 16c93f1c9c70d0d15b05bb81c8eb40701fc41e9b | ed59efd5f293102f3b5137ad1c38bb1ac460b187696dcbb704dce8574445f875 |
| Performance | 00cf3165ed712b2f964a9b0ab76c308ee555bd72 | 744afde06c059f0a9d6a43b39e0865fb6646bf4278402304e30a7aef9c492bf3 |

Previous candidates are preserved in
build/deko3d-test/diagnostics/2026-10-29-before-overlay-audit-pass/.

## Next test

Use build/deko3d-release/openrac-rac1-pal-deko3d.nro, explicit 480p, stock clocks,
same FPS cap and route. FRAME_COST off. Positive DRAW_TRACE/MIP_AUDIT values in
an existing env.txt override the new defaults, so remove them or set them to 0.
For a measured run, use the nonblocking CPU_TIMING/DK_TIMING/DK_COUNTERS guide.
Launch in full application mode through the same working homebrew launcher.

Collect openrac-rac1-pal.log and sdmc:/switch/openrac/overlay-status.log; include
the Status Monitor version and a photograph of its FPS/RES readings (Nintendo
captures omit them). If connection fails, use the recorded Result codes before
changing IPC, installing sysmodules or guessing the backend. If connection and
publishing succeed but the reader is blank, check reader/app-process discovery.

Do not claim the overlay or performance improvement is hardware verified yet.
The Pyrocitor preview material and landing audio delay remain open. No commits
or pushes were made.
