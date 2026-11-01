# Hardware session33: pass timing and pacing baseline

Session2026-10-30 02:41:59. Full game log archived in
build/deko3d-test/pacing-baseline-review/logs/openrac-rac1-pal.log.
Env from attachment5 confirms normal sampler1, lazy0, late acquire0, no-between0,
all timing/counts120 and no FRAME_COST. Internal640x480, picture960x720, output1280x720.
Pass reports are valid in observed windows: dropped0/rejected0/unbalanced0.

Heavy final120frame window:
- Scene span61.188ms, tail0.488ms, GPU inter-frame timestamp gap5.269ms.
- Whole command span61.676ms; present interval66.978ms; about14.9presents/s.
- Ties translucent17.834ms, ties solid14.911ms, ties cutout7.026ms:
  combined39.771ms (about65% of scene span in this window).
- Terrain10.855ms, shrubs4.067ms, mobys opaque3.972ms; other passes small.
- Top-level pass sum60.727ms; compares closely with scene span but includes
  present blit and omits work outside scopes. Do not equate with pure GPU busy time.

The large rendering cost is scenery/tie passes, not primarily Ratchet, particles,
metal or the final screen blit. Next source optimization should target tie draw
coverage/material passes, sampler state churn and conservative geometry visibility,
with reference switches and image comparison. Do not silently drop transparent
surfaces or remove correctness barriers. The tie translucent pass warrants checking
how much geometry is processed only to discard fragments; not proved waste yet.

Presentation gap exists (5–7ms in several heavy windows; varies elsewhere). This
supports trying late acquire, but does not prove acquire causes all of the gap.
Even eliminating5ms cannot make a61ms scene meet a16.7ms target. No evidence that
an overclock is strictly required; no stock-clock performance target guarantee.

Pacing: some windows render95or117between frames for120ticks (1.79/1.97presents/tick),
while every tick is behind schedule and no tick frames are skipped. Those extra
frames interpolate without advancing simulation. Other windows have0betweenframes,
so suppressing them cannot explain or fix every slow view. No >100ms restart is
needed for this to happen: counts are zero in those windows, refining Claude's
original inferred restart mechanism. Last line game-tick report:13.2ticks/s.

Copy attribution: gameplay~1.7–2texture uploads/frame,~2Full barriers/frame;
framebuffer copies/mipmaps0. Do not pursue framebuffer-copy elimination on this
evidence. First8busy-upload names are exhausted mainly by startup selftests/movie
512x416uploads, so they do not identify later gameplay uploads. Naming must be
sampled later or budgeted per phase to identify their source before optimizing.

Prepared build/deko3d-release/late-acquire-test/env.txt; exactly one line differs
from current baseline: OPENRAC_DK_LATE_ACQUIRE=1. No rebuild required; same NRO
already contains the experiment. Keep normal sampler1, no-between0, lazy0.
Repeat route and collect logs, env-applied, overlay and clip. Check gap/tail,
presentation interval and flicker. After that, test no-between separately with
late acquire0. Do not combine changes yet. No runtime code changes/build this review.
