# Session39 PVS — 2026-10-30

Evidence archived build/deko3d-test/pvs-session39/. User reports closer to30FPS,
cutscene smoother but still delayed. No claim of complete visual/culling validation.
Game activation confirms PVS,terrain batching,and behind-frame suppression ON.
Settings original distances,640x480,cap60. Attached env-applied remains stale or
mismatched; actual game activation/counters take precedence.

Selected gameplay windows before the ending scene transition:
GPU scene16.411–18.603ms; ties solid4.171–6.065ms,cutout0.939–1.306ms,
translucent0.801–1.111ms; terrain1.574–1.968ms. Earlier session38 selected heavy
windows scene37.796–38.794ms,ties total26.4–26.7ms. Different cameras/routes;
strong evidence PVS saves work, not a controlled percentage FPS benchmark.
Game reports30.2 then29.3 updates/sec. Pacing mostly120tickframes,0between,
120declined-behind. Gameplay display interval approximately33ms. GPU gap commonly
14–18ms, but it includes queue starvation/CPU scheduling and screen-buffer waits;
it must not be labeled all avoidable GPU idle or all acquire wait.
Game-side wall interval outside vsync approximately11–14ms includes waits/I/O;
CPU recording approximately7–9ms, not simply additive with GPU spans.

Reviewed existing late-acquire present path (scene submit/flush before acquire,
transfer fence retained, present blit and frame fence afterwards). Existing host
mock rerun with timing30+pass timing30+late acquire1:0failures. Not hardware proof.
Prepared pvs-late-acquire-test/env.txt: exactly one setting changed from session39
config, DK_LATE_ACQUIRE0->1. No NRO rebuild/source edits/default promotion.
Compare same stock-clock route; gameplay speed, frame pacing, flicker and landing.
If regressions, restore terrain-pacing-pvs-test/env.txt (session39 reference).
Cutscene delay remains unresolved; distinguish slowed game ticks from initial
stream offset. Need synchronized audio/animation trace if presentation experiment
does not resolve it; do not adjust global audio delay based on GPU timing alone.
