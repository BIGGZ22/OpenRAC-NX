# Pacing/pass timing build — 2026-10-30

Read CLAUDE_REVIEW_PACING_AND_PASS_TIMING_2026-10-30.md and the user's pasted
handoff. Reviewed late-acquire list ordering, transfer fence, end/slot fence,
pass pair reservation and Scope forwarding; set_tick has one caller, vsync.cpp.
No new runtime changes made in this build pass. Hypotheses about GPU idle time,
image release waits and game slowdown remain hypotheses until measured. GPU gap
is timestamp separation, not a pure utilization counter.

Real-renderer mock compiled and passed all14 clean-process settings requested:
default, late acquire, pass timing, combined pass/late/GPU timing, GPU timing120
and30, texture runs off, sampler cache off, lazy barriers on, vertex cache off,
panel trace1, sampler first/new, counters30. Unit sampler-refresh and moby-pass
tests passed. stats_pass_hooks_test passed linked against real stats.cpp/env.cpp
and the existing host log stub. The first standalone link omitted dependencies;
adding env.cpp/log stub resolved it. No test failures remain. New hooks test not
registered in a CMake list during this build-only pass; future test integration
remains optional follow-up.

Both Switch inner logs report exit code0 and built openrac-rac1-pal_nro. Copied
NRO/ELF hashes match build outputs, patched/stock packs match staged RomFS. ELF
contains the requested gap/pass/copy/pacing/experimental and default-path strings.
No pack regeneration, commit or push. No new hardware verification yet.

Performance: build/deko3d-release/openrac-rac1-pal-deko3d.nro
SHA256 d6b717e159b2acb133af55981ba063f1fc7efa08168873b677333429afcfe0ef
ELF buildID77b43a8ced71d1d9e6a343e48257d4138cf279b3
Diagnostic: build/deko3d-test/openrac-rac1-pal-deko3d.nro
SHA256 eaf68440435f30c907e77c10ebeec5790726d5c3664828517680b0ffd54f4b06
ELF buildID8f3969fb032a5eb9ce791d3d6109e971af558d13
Matching ELFs/manifests stay in each output directory; hardware_tested=false.
Prior builds and baseline env preserved in diagnostics/2026-10-30-before-pacing-pass-build/.

Ready config is build/deko3d-release/env.txt. PASS_TIMING/PACING_COUNTS120 added.
Normal sampler refresh1 for the fresh baseline (not session32's first mode).
Late acquire0, no-between-when-behind0, lazy target barriers0; FRAME_COST absent.
Test640x480, stock clocks, same cap/Metropolis rotation and flight/landing. Detailed
logging off. Collect game, env-applied, overlay-status logs and Status Monitor clip.
Pass timestamps may perturb timing; compare future experiment runs with the same
instrumentation enabled. No extra CPU GPU wait is deliberately added by readback.
After baseline, change only late acquire, then separately no-between; combine
only if individually useful and visually clean. Test sampler first later.
