# Session36 terrain experiment, compared with session35

Game log explicitly confirms TERRAIN_BATCH=1 and batch snapshots, despite attached
env-applied log lacking that variable. Treat applied-env attachment as incomplete
or mismatched; activation is proven by renderer log, not inferred from filename.
Both game logs use640x480/original distances. No identical-camera controlled A/B.

Selected late windows: baseline terrain9.282–10.188ms, solid1420–1552draws and
36260–38098triangles; batch terrain2.938–3.361ms, solid49–55draws with
36028–36848triangles. CPU record baseline15.154–16.175ms vs9.065–10.676ms.
GPU scene baseline44.457–45.789ms vs36.990–38.936ms. Strong evidence terrain
batching reduces work, without claiming precise same-scene percentage speedup.

Presentation remains about50ms (~20presents/sec). New pacing windows draw120tick
frames PLUS60between-tick frames while all120ticks are behind. At50ms/present,
180presents take9s => about13.3game ticks/sec, consistent with frame2400 log.
Baseline late window had120tickframes and0between frames. The CPU recording
improvement appears to make extra interpolation frames eligible while the GPU
and presentation remain slow. This is a pacing hypothesis supported by counters,
not a hardware-tested fix. Static ties still cost about26ms across three passes.

Prepared terrain-pacing-test/env.txt: retains terrain batching, changes only
NO_BETWEEN_WHEN_BEHIND from0to1. Uses existing NRO. Expect fewer extra frames;
measure game ticks and movement/audio timing as well as displayed FPS. It cannot
alone make a37ms GPU scene meet a33.3ms budget. Other experiments remain off.
No source edits or build in this analysis. Exit result still not confirmed.
