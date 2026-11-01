# Scenery/pacing build — 2026-10-30 03:12 EDT

Read CLAUDE_REVIEW_SCENERY_PACING_2026-10-30.md and pasted handoff. Reviewed
translucent culling against existing camera survivors, preserved survivor order,
invalid-box retention, group-index use and opt-in opaque sorting. No shader changes.

Codex corrections before build:
1. Registered StaticTranslucentCullOff in env.h/env.cpp's ordered table and documented
it in DEBUGGING.md. stats::probe otherwise warned and stayed false for this new
name, making the A/B reference impossible. Env table/documentation tests pass.
Truth::Set means ANY set value enables this reference, including0; remove it for
optimized behavior. Baseline env intentionally omits it.
2. test_logs now captures append offsets with prepare() immediately after env load,
before launcher/overlay initialization. begin() uses these offsets. Crash recovery
caps old append tails at those early offsets so current startup messages aren't
misattributed to the previous crashed run.
3. Log archive recovery only accepts timestamp-named direct children of its root;
pruning ignores unrelated folders and symlinks. latest cleanup replaces known
archive filenames without recursive deletion of user content. Production SD path
unchanged. Compile-time OPENRAC_TEST_LOG_ROOT permits isolated native Windows tests.
4. Host test uses a normal test directory instead of an invalid Windows sdmc: path,
and normalizes CRLF in text assertions. Extended tests cover early overlay capture,
recovery boundaries, traversal rejection and preservation of unrelated folders.

Tests: real-dkgl mock zero failures in14 clean modes (default, counters1/30,
late acquire, pass timing, combined late/GPU timing, GPU120, bind-runs off, sampler
cache off, lazy on, vertex-cache off, panel trace, sampler first/new). Native
stats-pass-hooks, sampler-refresh, moby-pass and env tests pass. Log archive test
passes in a fresh workspace temp directory. Scene renderer syntax check passes.
Initial archive test exposed Windows text newline assumptions; corrected fixture.
No unresolved test failures. No assertion-disabled test binaries used.

Both Switch builds inner exit0 and NRO target complete. NRO/ELF copies verified,
all patched/stock pack files match staged RomFS. Requested ELF strings verified.
No pack regeneration, commit or push. Hardware testing pending.

Performance NRO: build/deko3d-release/openrac-rac1-pal-deko3d.nro
SHA256 0f114116b78936c798b7b54bdc42c78ab91085b918ea417bb8f0e74dbf3e736c
ELF buildID1ed1d0c7cbeee1c4dc9ffe5dfdc71568109f0503
Diagnostic NRO: build/deko3d-test/openrac-rac1-pal-deko3d.nro
SHA256 8dafb2d0e1f28ccbb82ac5a2face35c0d71543a71826cbc57e49e63aa8bd743c
ELF buildIDb3766e533f46a9f235eeb5df2d2eebd86171540f
Matching ELF/manifest in each folder. Previous build+env preserved in
build/deko3d-test/diagnostics/2026-10-30-before-scenery-pacing-build/.

Ready env.txt beside performance NRO: new culling default enabled; sampler1;
late acquire0, no-between0, between-present-cost0, front-to-back0, lazy0;
all reports120, TEST_LOGS1, FRAME_COST absent. Launcher settings remain user's.
Test640x480, stock clocks, same FPS cap/route. Observe glass, fences and foliage
for culling errors. Compare ties translucent timings and survivor counts with
session33; do not claim gains before evidence. Then test one experiment at a time.

Exit via L+R+Minus (the game's std::exit route) to collect test-logs/latest/.
A HOME force-close/power loss may not run atexit; recovery occurs at next game
start into the timestamped previous-run folder with NOTE.txt, not necessarily
latest/. Atexit/SD archive behavior itself remains to be confirmed on hardware.
Archive copies only known logs; COMPLETE currently marks an archive attempt,
not proof every optional source was present. Keep originals for this first run.
