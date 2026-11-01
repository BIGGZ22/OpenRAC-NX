# Terrain and exit candidate — Codex review/build 2026-10-30

Both builds completed openrac-rac1-pal_nro with inner exit code 0. Copied NRO/ELF
hashes match build outputs; manifest build IDs were read from matching ELFs.
Patched and stock shader pack source files match staged RomFS; no regeneration.
Previous artifacts/env/manifests preserved and hash-checked under
build/deko3d-test/diagnostics/2026-10-30-before-terrain-exit-build/{release,test}.

Performance NRO SHA256: 3e357444a9e129170db3d789345d05e3b1c741b67f8879986ed324b1c766082f
ELF build ID: 93fd2a7c08f2081c6076613451c5d4d3483b6e83
Diagnostic NRO SHA256: fe1e73794a7664bd8bc50139bb72c822ab3201998203e2ef96c2fc035167e07b
ELF build ID: e9d50c1980df17453c7790d52bb193aada436250
Hardware tested: NO. Compiler warnings remain; build success is not warning-free.

Reviewed batch eligibility, signature, bounds fallback, element-buffer restoration,
release cleanup and pass-report pair reservation. Experiments remain default off.
Terrain cost/draw count suggests draw overhead; it does not prove a draw-bound
bottleneck or predict speedup. Material reordering can change equal-depth results.
Pass counters may be unsupported/zero; invocation/sample ratios also depend on
hardware counting semantics, not solely discarded fragments.

Host validation: freshly compiled terrain helper, sampler refresh, moby passes,
log archive (new isolated fixture), stats hooks and env tests passed. Mock passed
all 17 requested supported modes. Disabled sampler refresh intentionally fails1;
legacy transfer mode intentionally fails3, as previously documented.
Actual Switch/AArch64 syntax-only vsync check passed with __SWITCH__ and frontend.

Codex change: env_test's documentation check now explicitly verifies ten documented
Switch direct-getenv readers against their source files. Previously it incorrectly
required those variables to be in the central table. Runtime settings unchanged.

Verified ELF contains terrain experiment, terrain batch snapshot, pass statistics,
and late-tick image-send diagnostic strings. No game-source changes beyond Claude's
handoff and prior Codex exit fix during this review. No commit or push.

First test: performance NRO plus baseline env, stock clocks,640x480,original draw
distances,same cap/route. Save then test shortcut exit; separate run HOME close.
The HOME appletMainLoop path may not execute on forced termination; do not assume
an orderly archive will always exist. Preserve logs after each run before next.
Next test: terrain-batch-test/env.txt, same scene; compare terrain draws/time and
watch seams or missing surfaces. PVS and other experiments in separate runs only.
Detailed launcher logging off; FRAME_COST absent. See Claude review for full plan.
