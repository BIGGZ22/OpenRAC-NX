# Session44 build review — 2026-10-30

Release and diagnostic builds completed openrac-rac1-pal_nro and inner exit0. Copied NRO/ELF hashes match outputs; build IDs verified in last-build.json. Both source shader packs match staged RomFS. No shader pack regeneration, commit or push performed. Previous tested candidates preserved under build/deko3d-test/diagnostics/2026-10-30-before-session44-build/.

Release NRO30fa1edac88644b2e6cd52c1d830ccc802317accded0f60a97c8c56bf75abc1b; ELF2527e0c0f4294668a1d34f823f1fd197620665e8.
Diagnostic NRO89081c36ecae5914e235802f7f7c0e19fb8826ae485cc0dceb105a8a096dff7f; ELFcf57cd4a28f1aca44ca089c8d2a32031ce48627f.

Review: native/1/ path maps moby texture table; cutout/material bounds guard and opt-in old mipmap reference retained. Frame cap uses actual recording start while CPU cost retains shifted start. Synchronous starts coincide. Delayed first frames count in between() as tick frames. Repeat verdict retains conservative scan checks; fixture limited1MiB,one write. Pool placement excludes creator but can share the render worker's core: opt-in hardware experiment, not guaranteed speedup. Queue tracing reads under lock and only explains ordering; no audio semantics change. No hardware visual correctness claims.

Windows direct checks: moby_passes,image_sends,tick_phases(default and=30),parallel,env all passed. Thread/timing checks use test-only logging/crash setup stand-ins, no sanitizers. Full Switch builds compiled changed actual translation units. Existing warnings remain.

Codex adjusted ONLY sound_stream_early_test's racing-reader loop: the old1000offline-render-iterations could finish before the background reader got CPU and contaminate subsequent queue assertions. New bounded10second wall deadline with1ms sleep per10ms simulated audio chunk gives reader scheduling time. Runtime audio source not changed by Codex. Initial early-on test failed stream completion/sample/queue checks before this adjustment; record rerun separately below.

First hardware run: new performance NRO+build/deko3d-release/env.txt (copy of session44-tests/01-worker-fixes). Check vendor lettering, heavy gameplay, landing,HUD/pause/map and normal exit. Send latest logs plus sdmc:/switch/openrac/image-sends-fixture.bin if present. Then02-pool-cores same route;03-references only to resolve unclear visuals/cap behavior. Early stream start is off in run01 (based on session42). Preserve prior three-run configs.

Audio rerun: all four early-start off/on x audio-trace off/on combinations passed including queue/loop assertions after the test scheduling correction. All pinned hashes2bd2a9fa9628097b,497426sounding samples. Hardware remains unverified.
