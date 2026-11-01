# Hardware session 41: scheduling baseline, 2026-10-30

Logs archived in build/deko3d-test/baseline-session41/. User identifies performance NRO with baseline env. Effective environment confirms terrain batching, PVS and behind-frame suppression on; DK_RENDER_THREAD, LATE_IMAGE_REPEAT_SKIP and STREAM_EARLY_START absent/off. FRAME_COST false. Overlay attaches successfully and publishes 640x480 game rendering to 1280x720 output; brief 512x416 earlier target is also reported. Clocks cannot be established from these logs.

Representative heavy windows: 28.9-29.9 game ticks/sec, whole tick 33.5-34.6 ms. Game 13.1-13.9 ms, capture 6.8-7.9 ms, render CPU 8.5-8.9 ms, present roughly 0.9-2.35 ms, sleep 2.5-2.9 ms. GPU scene approximately 18.9-19.8 ms, command span approximately 19.4-20.3 ms, queue gap approximately 14-15 ms; these are overlapping CPU/GPU timelines, not additive busy times. Lighter late windows reach 37-41 ticks/sec. Not a matched-camera comparison to session39.

Audio trace sector1555962 proves growing drift: after 20.27 seconds wall time, mixer played20.26 seconds but game advanced600 ticks=10 seconds; audio ahead+10.26seconds. After26.43seconds: audio26.42/game14.00/ahead12.42. Request-to-first-sound wall interval7.085seconds is not by itself proof of decode delay; distinguish scheduling/queued playback and decode timestamps before attributing it.

Important new blocker: every checked heavy-window late image path is not decoded cleanly (100/100,101/101 etc). Therefore repeated-image skipping must conservatively refuse those paths. Run02 can still measure the render worker but may get no contribution from repeat skipping. Investigate decoder rejection reason after collecting run02; do not relax safeguards blindly.

Next: same performance NRO, three-run-tests/02-rendering/env.txt, same route and clocks. Check dedicated worker/core log, ticks/sec, worker wait, queue gap, HUD/vendor/map/transitions and exit. No rebuild or source change made for this analysis. Log does not establish clean exit or fix vendor/canopy issues.
