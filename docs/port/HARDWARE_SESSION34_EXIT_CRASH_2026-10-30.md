# Session34 scenery baseline and exit crash — 2026-10-30

Inputs archived under build/deko3d-test/scenery-baseline-review/logs/.
Run starts03:14:57. Env confirms default translucent culling, normal sampler1,
all experiments0, logs1 and FRAME_COST absent. Graphics line confirms ORIGINAL
DRAW DISTANCES, nearest filter0/anisotropy0auto,1sample,640x480,vsync0,cap60.
Do not assume this run used full-detail distance; session33 settings were unknown.

Culling verified active by snapshots. Frame3480: ties1786camera-kept of5081total,
terrain723of1006, shrubs141of363. Ties translucent35draws150716instancedtriangles.
Near final heavy window: scene47.673ms, ties solid14.992ms, cutout6.899ms,
translucent6.002ms, terrain10.984ms; present55.916ms. Session33's similar heavy
window had scene61.188ms and translucent17.834ms with solid14.911/cutout7.026.
This is promising attribution, NOT an identical-camera/scenery-settings A/B or a
proven FPS percentage gain. Await clips for visual regressions/canopy/audio.
Pacing still identifies 2D-list image uploads as the reason many late tick frames
are rendered, suggesting further pacing/upload-source work.

Crash: Atmosphere01793344622_03db12780bd84000.log, Data Abort/null fault.
PC is sphaira+0x2f3c58, LR+0x2f3a9c. Only module is sphaira, buildID
80A1B35C3F1F5E9676C24A76AA0AF13213E9F938 (not game ELF1ed1d0c7...).
Cannot decode those addresses against the OpenRAC ELF. The report locates the
fault in loader code, but does not exclude game-induced heap corruption or
shutdown-order trouble. Game log ends with3613frames, so finish's atexit callback
ran. Do not claim log archiving or Sphaira alone caused the crash.

Concrete source issue found/fixed: Switch L+R+Minus in program/vsync.cpp called
std::exit directly, bypassing the existing leave() path. leave() explicitly closes
audio before SDL teardown, closes frontend/joins rendering, stops sound readers,
then exits. Existing comments already document heap corruption on wrong audio
teardown. Shortcut now calls leave() when a frontend window exists; headless
fallback stops readers before exit. No bypass of driver checks or filesystem
archive added. This corrects an unsafe path, NOT a hardware-proven crash fix.

Host and actual Switch/AArch64 -fsyntax-only checks of vsync.cpp passed, including
__SWITCH__ and OPENRAC_FRONTEND branches. No NRO/ELF rebuilt in this log review;
current output files do NOT contain the fix. Preserve matching crashed artifacts.
Do not use shortcut for now. A future exit test should compare with/without
OPENRAC_TEST_LOGS and another loader only if the orderly cleanup still crashes;
obtain a matching Sphaira ELF/version if exact loader symbols are needed.

## Video follow-up: compressed_25MB_18566.mp4
73.493 seconds, phone recording. Sampled at six-second intervals; full-resolution
frame inspected at64s. Evidence saved under scenery-baseline-review/video/.
At64s overlay reads PFPS18/FPS18.9, GPU93.7% at307.2MHz, CPU1021.1MHz,
1280x720 output /640x480 render. CPU cores approximately37/34/0/30% at that
instant. Supports a GPU-heavy scene bottleneck alongside logged47.673ms scene
span, not a claim that every scene is GPU-limited or CPU optimization is useless.
Sampled scenery/model frames show no obvious catastrophic geometry corruption;
sparse samples cannot rule out transient culling/popping or canopy displacement.
Audio sync was not evaluated from still frames. This clip supplies no new proof
of exit-crash root cause. No build or source changes made for this video review.
