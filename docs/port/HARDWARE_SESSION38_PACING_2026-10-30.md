# Session38 — pacing suppression active

Archived at build/deko3d-test/terrain-pacing-session38/.
Game counters confirm120tickframes,0betweenframes,120declined-behind per late
window. Attached env-applied still says0 and lacks terrain switch: this attachment
is not authoritative for actual runtime; do not ask user to repeat this test.
Heavy windows scene37.796–38.794ms, terrain3.108–3.345ms, ties three passes
26.423–26.682ms, display49.9ms. Final game rate19.9ticks/sec versus13.5 in37.
Comparable workload ranges, not identical-camera A/B. Suppression removes wasted
interpolation work and improves game update rate, not displayed heavy-scene FPS.
No evidence audio synchronization is fully fixed. Exit result still unknown.
Next independent variable: existing PVS, retaining terrain batching and suppression.
Prepared build/deko3d-release/terrain-pacing-pvs-test/env.txt. Watch missing/pop-in
scenery at camera/cell boundaries. No rebuild. No default changes/source edits.
