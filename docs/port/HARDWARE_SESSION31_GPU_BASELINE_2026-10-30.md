# Hardware session31 — reviewed GPU pass baseline

Inputs archived under build/deko3d-test/session31-review/logs/; clip contact sheets
in that parent directory. Session starts2026-10-30 01:44:52. Internal640x480,
output1280x720, picture960x720. Env confirms glow skip1, texture bind runs1,
sampler rebind1 (normal workaround), lazy barriers0, FRAME_COST absent.
Canopy flight-at-tick log appears. No new error in startup log (SDK0.5).

Optimization mechanisms are active: per-pass snapshots report23–29groups ruled
out per glow step in representative Metropolis frames. These are eligibility
counts before visibility/blend tests, not measured saved draws. A frame3600snapshot
has draws67/5/5/4/1/3 across solid/glow-depth/glow-blend/translucent/effects/metal.
Binding totals at the last report:4,219,830commands for4,341,052stage-slots, about
2.79% fewer binding calls than independent single-slot calls. Not an FPS gain.

Sustained slowdown remains. A heavy120frame window:2164draws/frame,
633.4sampler-set refreshes/frame; GPU command span62.009ms, present interval66.648ms,
CPU record14.834ms, slot-fence wait39.757ms. Other heavier views span52–62ms with
presentation60–67ms. These are different route samples, not identical-camera
A/B comparisons with session30; no overall percentage speedup established.
Command span is not GPU busy time. Sampled clips retain coherent scene geometry;
contact sheets do not prove all frames, canopy attachment or audio synchronization.
Overlay connection and FPS/resolution publishing remain functional.

Next isolated experiment prepared at build/deko3d-release/sampler-first-test/env.txt.
It differs from baseline config by exactly one line:
OPENRAC_DK_SAMPLER_REBIND=first
No rebuild required; the NRO already supports this opt-in experiment. Normal
baseline env.txt beside the NRO is unchanged and is the rollback config.
Repeat640x480, same cap, clocks and Metropolis rotation/landing. Inspect for stale
colours/filter/wrap, especially after loads. Revert baseline if new artefacts appear.
Do not try new mode until first is visually clean. Collect per-run logs separately.
No new source edits/builds in this review.
