# Session 55 release checks (2026-11-01)

Reviewed diagnostic-off parsing, tick-frame tolerance, render-worker completion before the behind decline, and ring logging. `ready_until(next_tick)` uses the same deadline that the game thread subsequently sleeps until. The half-step tolerance does not alter the cap schedule or simulation tick rate. The reference switch retains the previous behaviour. No shader regeneration or renderer synchronization changes were made in this pass.

Seven focused Windows host tests passed after recompilation: env_interval, frontend_pacing (including model scenarios), frame_pacing, env, tick_pacing, switch_clocks_mock and moby_pose. Pose verification compared 6000 random mobys and 3000 transitions with zero differences; reused scratch averaged zero allocations after warm-up. These are host checks, not Switch gameplay verification. The model measures CPU present calls, not scan-out.

The initial Switch release build completed `openrac-rac1-pal_nro` with inner exit code 0. A final build from the release commit is required before publication. The existing root performance and diagnostic NROs, ELFs, manifests and shader packs were hashed before building and remain unchanged. Output and verification records are kept separately in `build/deko3d-release/session55/`; no binaries, logs, game data or test env files are committed.

The release includes previously uncommitted renderer, launcher and upstream fixes as well as sessions 48–55: pose reuse, PAL 50/60 timing, optional clock lifecycle, logging location and diagnostic-off semantics, and tick-frame pacing. Native shader packs contain compiled project-authored renderer shaders with provenance; no runtime shader dumps or disc-derived assets are included in the source commit. Their bytes are preserved.

Hardware status: **not tested** for the session-55 candidate. Keep the two session55-tests configurations local. First compare clean reference pacing against new pacing on the same route, clock setup and cap; exit with L+R+Minus to dump the in-memory ring. Use `port/tools/pacing_ring_summary.py` on each log. Try EVEN_PRESENT only as a separate follow-up.
