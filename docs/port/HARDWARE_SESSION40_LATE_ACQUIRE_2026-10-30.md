# Session40 late acquire — 2026-10-30

Logs archived build/deko3d-test/late-acquire-session40/. User reports same feel.
Actual game activation confirms late acquire1, PVS1, terrain batch1, suppression1.
Selected sustained gameplay windows GPU scene18.343/19.583ms, gap14.697/13.958ms,
present tail0.509/0.507ms; pacing32.0/32.6ms per frame and120tickframes with0between.
Game reports30.0 near3300. Session39 gameplay scene16–19ms,pacingabout33ms.
No demonstrated practical improvement; routes not exact A/B. Keep late acquire off
in reference config (terrain-pacing-pvs-test/env.txt). Do not interpret ending
scene/menu37.8FPS as a comparable gameplay gain.

Gap did not shift predominantly into present tail. Weakens original screen-acquire
wait explanation for current bottleneck. Gap can include CPU production/scheduling;
not measured GPU busy time. Game outside-vsync interval13.3ms and CPU record8ms
in selected windows; overlap/waits require timeline investigation before summing.
Next source investigation: game/render scheduling and late tick image-transfer
handling; fix suspicious diagnostic packet decoding before using transfer values.
Landing audio delay unresolved; distinguish wall-clock audio ahead of slowed game
from fixed start offset, use existing audio trace if needed. No new build or game
source changes for this review. No need repeat late-acquire test.
