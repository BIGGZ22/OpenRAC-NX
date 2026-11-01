# Reviewed GPU pass build — 2026-10-30 01:42 EDT

Read Claude's handoff and CLAUDE_REVIEW_GPU_PASS_2026-10-30.md. Reviewed sampler
refresh default and experiment dispatch; baseline behavior remains unchanged.
Ran the real-renderer host mock: default, bind-runs off, sampler-cache off, lazy
barriers on, DK_TIMING120, vertex-cache off, panel-trace1, sampler first and new:
all zero failures when environments are clean. Also sampler-refresh and moby-pass
unit tests pass. An initial sequential test runner left empty environment variables
behind; that triggered getenv-based fixture branches and false failures. Reran
those modes with Remove-Item Env:<key> before each process; all passed. No renderer
change was needed to resolve this harness issue.

Built both targets. Inner exit code0 and NRO target complete in both logs. Copied
NRO/ELF hashes match build outputs; all patched/stock pack files match staged
RomFS. ELF contains EXPERIMENT first, EXPERIMENT new and dk texture binding totals.
No shader regeneration, commit or push. No new hardware validation yet.

Performance NRO: build/deko3d-release/openrac-rac1-pal-deko3d.nro
SHA256 ed90521b4a96c6be82ae0b136c768135c73eb6fc915de1277b3102d9238244a9
ELF buildID eeccf8f60d91620a79a352625ca470329b453b2f
Diagnostic NRO: build/deko3d-test/openrac-rac1-pal-deko3d.nro
SHA256 0d18b12a7001d47c43b5745c156d0dd6e00b28e741ec03eeb268992a08e05c58
ELF buildID a9e35c381bfc48a8b03b3753bf33650ef210883a
Matching last-build.elf and last-build.json preserved in each output folder.
Previous builds: diagnostics/2026-10-30-before-gpu-pass-build/{release,test}/.

Ready baseline config: build/deko3d-release/env.txt. Includes passive/game timing,
scene pass counts120, glow skip1, bind runs1, sampler rebind1, lazy barriers0.
FRAME_COST absent. First test same640x480, stock clocks, cap and Metropolis route
as session30. Do not turn sampler experiments on for baseline. Collect game log,
overlay-status.log and Status Monitor readings. Compare one switch at a time later.
No additional per-pass GPU timestamps or engine-copy source counters implemented;
these remain follow-up measurement candidates, not missing build prerequisites.

Session30 logs now copied for future reviews into
build/deko3d-test/session30-review/logs/ (game and overlay logs).
