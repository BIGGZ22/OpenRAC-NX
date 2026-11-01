# Session45 hardware: repeat-skip-live

Archived logs: build/deko3d-test/repeat-skip-session45/. Effective env confirms repeat skip+worker+terrain/PVS; strict reference unset; pose helper unset, pool reports0/hardware0. User supplied logs with no subjective/visual report.

Decoder parity change is active: sampled heavy-window verdicts unread0,incomplete0,mostly different sends110-116,repeat0. Final transition/pause window has repeat5,all5with renderer-dropped tags,offscreen1,different97. Therefore skipping does work in eligible cases but most heavy gameplay uploads genuinely compare different under current full ordered-path equality. Do not weaken hash/rectangle/ordering comparisons to manufacture skips. Next useful diagnostics would identify first mismatch and which transfers change, using bounded aggregates; no reason to assume repeated HUD path was dominant unchanged work.

Heavy samples35.7-41.4ticks/sec,whole tick24.2-28.0ms,game14.2-16.2ms,prepare3.9-7.8ms,capture2.5-3.2ms,workerwait~zero. GPUcommand10.6-13.7ms,directpresent23.5-28.6ms. Display/tick1.00 except finalpause/transition0.94. Previous44selected~41-47; no controlled same-camera/clock comparison, cannot declare regression caused by patch. No isolated demonstrated FPS benefit from repeat skip.

Audio drift still grows; pause present, so no wall-time delta comparison without tracing actual playback/game paused state. No new source/build changes. Next sameNRO+session45-tests/02-pool-one-helper/env.txt,stockclocks640x480same route. Check onehelper/core placement,preparecost,ticksandpresentations,HUD/vendor/map/exit. User hasn't reported visual correctness for this run; do not mark broad graphics tested.
