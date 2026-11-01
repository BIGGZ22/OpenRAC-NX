# Claude review: sessions 30–32 pacing, and new GPU attribution (2026-10-30). Source only, NOT BUILT

**Nothing here was built or measured on hardware.**
- Claude did not build the game, did not regenerate shader packs, and made no commit and no push.
- The current NROs (performance `ed90521b…`, build ID `eeccf8f6…`; diagnostic `0d18b12a…`) do
  not contain these changes.
- Every result below is from the session logs, host compiles and the mocked-deko3d test.

What this pass adds:
- **Measurement only:** GPU gap and scene/tail split; per-pass GPU timestamps; copy-engine and
  Full-barrier sources; pacing counts.
- **Two opt-in experiments:** `OPENRAC_DK_LATE_ACQUIRE=1` and `OPENRAC_NO_BETWEEN_WHEN_BEHIND=1`.
- **Default rendering, ordering and pacing are unchanged.** The only default-path change is
  one timestamp report at the scene's end, recorded only when `OPENRAC_DK_TIMING` is set.

Preserved untouched:
- the patched UAM pack;
- the SDK 0.5 vertex-binding and texture-invalidation workarounds;
- Full copy-engine ordering;
- the default sampler-refresh rule;
- lazy barriers off;
- saves, launcher, language persistence and canopy world capture.

## 1. What the logs say

Logs re-read:
- `session30-review/logs/`: baseline, before the GPU pass;
- `session31-review/logs/`: `SAMPLER_REBIND=1`;
- `sampler-first-actual-review/logs/`: session 32, `SAMPLER_REBIND=first`.

All three runs: 640x480 internal, 1280x720 output, stock clocks, `FRAME_COST` absent, lazy
barriers 0.

Windows are 120 frames. Column meanings:
- **span:** `dk timing` GPU command span, top→end. This is not GPU busy time.
- **present:** CPU present-to-present interval.
- **fence:** CPU slot-fence wait.
- **record:** render CPU record time.
- **game:** game wall interval outside vsync.

Selected Metropolis windows:

| run | span | present | ceil(span/16.7)·16.7 | fence | record | game | draws | refreshes |
|---|---|---|---|---|---|---|---|---|
| 31 | 21.0 | 33.3 | 33.3 | 23.0 | 5.2 | 1.6 | 726 | 343 |
| 31 | 36.5 | 50.0 | 50.0 | 30.1 | 5.5 | – | 857 | 341 |
| 31 | 54.2 | 61.7 | 66.7 | 34.7 | 12.5 | 12.8 | 2164 | 633 |
| 32 | 29.9 | 33.4 | 33.3 | 14.2 | 5.0 | 11.2 | 460 | 7 |
| 32 | 36.0 | 49.9 | 50.0 | 27.4 | 7.9 | – | 1085 | 7 |
| 32 | 31.7 | 33.3 | 33.3 | 10.3 | 5.6 | 11.0 | 1591 | 7 |
| 32 | 42.9 | 50.0 | 50.0 | 21.0 | 14.4 | 12.1 | 1473 | 7 |

### Findings

1. **Presentation is rounded up to whole refreshes.**
   - Across sessions 30–32, the present interval tracks `ceil(span / 16.7 ms)` far more closely
     than it tracks span. Spans of 21–32 ms present at 33.3 ms; 36–45 ms at 50 ms.
   - That explains why `first` cut the GPU span 18 % (62 → 50.6 ms) but presentation only 3.8 %:
     both runs sat in the same refresh bucket.
2. **The GPU is idle part of every frame, and the idle time is not in the span.**
   - The CPU waits 14–35 ms per frame on the fence of the frame *before* the one it just
     submitted. So the next frame is always queued when the GPU finishes one.
   - The CPU therefore never starves the GPU. Yet the present period (e.g. 50 ms) exceeds the
     span (36 ms).
   - In steady state, the GPU's frame period equals the present period. So ~10–14 ms per frame is
     spent on neither frame's commands.
3. **The likely cause, confirmed in the deko3d source** (`dk_swapchain.cpp`, `dkQueueAcquireImage`):
   - acquire calls `nwindowDequeueBuffer` and then `queue->waitFence(fence)`, a queue-level wait
     on the image's release fence;
   - dkgl `present()` acquires *before* submitting the frame's list;
   - so the release wait sits in front of the **whole scene**, not only the final blit.
   - If the image dequeued is released only when the previous frame reaches the display (which the
     quantisation implies), every frame waits for a refresh before its scene starts.
   - `top` is written after that wait, which is why the span excludes it.
   - **This is a hypothesis.** The new `GPU gap` metric (section 2.1) measures it directly.
4. **Game speed and presents.**
   - In Metropolis the game ticks ~15–16 times a second against a 60 Hz tick rate, while presents
     run ~20–22 a second. So more frames are presented than ticks are drawn.
   - Each present is a full GPU frame.
   - Possible mechanism (inferred from `vsync.cpp`): once a tick is more than 100 ms behind,
     vsync restarts its count and `late` reads 0. That tick's frame is drawn, and `between()` can
     then draw an extra frame before the next tick.
   - The new pacing counts (section 2.4) measure what actually happens.
5. **Recurring transfers in gameplay** are ~1.5–1.8 copy-engine copies/frame, equal to texture
   uploads. There are 2 Full barriers/frame. The mock's palette case suggests a per-frame re-upload
   into a busy texture (one Full before, one at the transfer fence). Section 2.3 names them on
   hardware.
6. **CPU per tick is ~11 ms game plus ~5–14 ms record.** Well under the GPU spans; not the limiter
   in these windows.

## 2. Changes, with reasons

### 2.1 GPU gap and scene/tail split (dkgl, `OPENRAC_DK_TIMING`)

- **New timestamp:** a fourth report, at timing offset +48, written at the scene's end just before
  `present()` acquires the image.
- **New log line after each `dk timing` line:**
  `dk timing pacing: GPU gap end->next top …; scene top->scene end …; present tail scene end->end …`.
  - **gap** = this frame's `top` minus the previous frame's `end`. Frames are read in order after
    their own fence. A `top` before the previous `end` counts as "overlapped".
  - **Expected if the theory holds:** gap ≈ present − span (e.g. ~14 ms in 36 ms windows).
    A gap near 0 would rule the theory out.
- Existing timing semantics are unchanged; the timing block is cleared to 64 bytes instead of 48.

### 2.2 Late-acquire experiment, `OPENRAC_DK_LATE_ACQUIRE=1` (off by default)

- **What it does:** in `present()`, the scene's list is finished, submitted and flushed (after
  `transfer_fence`). Only then is the image acquired, and the barrier, blit, end timestamp, fence
  and present go in a second list.
- **The effect:** the queue's release wait now sits between scene N and blit N, and scene N hides
  it, instead of sitting in front of scene N+1.
- **Ordering:**
  - the blit's existing Fragments+Image barrier (wait for idle) still orders it after the scene;
  - the WFI is engine-wide, so it is valid across lists;
  - the slot fence still follows everything;
  - `reset_bound_state` and the new list's `begin_list` re-establish state, as at a list break.
- **Expected:**
  - present intervals move from the bucket ceiling toward the span, mixing 33/50 ms refreshes;
  - FPS rises with more refresh-to-refresh variation;
  - in the pacing line, the wait moves from `gap` into `present tail`.
- **Risks:**
  - one extra flush per frame;
  - a mid-frame L2 flush/invalidate from the second `begin_list`;
  - more uneven frame times at swap interval 1.
  - **Do not make it the default without a console A/B.**

### 2.3 Copy-engine and Full-barrier sources (dkgl, `OPENRAC_DK_COUNTERS`)

- **The tags:**
  - `after_transfer`/`before_transfer` take a `CopySource`: upload, mipmap, texture copy, blit
    colour, blit depth, other.
  - Full barriers are counted by cause: before a transfer (busy image), transfer fence, mipmap,
    readback, other.
- **New log line:** `dk copy sources per frame: …; Full barriers per frame: …`.
- **Naming the busy uploads:** the first 8 uploads into a busy texture are named
  (`dk copy: upload into a busy texture WxH format F …`).
- **Ordering is unchanged.**
- One counting difference: a framebuffer blit that copies colour *and* depth on the copy engine
  now counts as 2 engine copies instead of 1. It is accounting only.

### 2.4 Per-pass GPU timestamps, `OPENRAC_DK_PASS_TIMING=N` (30..3600; off by default)

- **The hook:** `common/stats.h` gains `PassHooks`. Every `stats::Scope` with `kGpu` calls them,
  whether or not stats are enabled. Not installed, the cost is one null-pointer test.
- **Installation:** `platform/switch/graphics.cpp` installs dkgl's `pass_begin`/`pass_end` when
  pass timing is on.
- **Existing sections covered:**
  - each sub-renderer (`renderer.cpp`: `r->name()`);
  - "world opaque colour" and "world depth prepass";
  - `level_renderer` pass steps ("ties solid/cutout/translucent", "mobys opaque/glow/translucent/
    effects/metal", "sky colour", shadow passes);
  - plus dkgl's own "dk present blit".
- **Each mark** is a deko3d `DkCounter_Timestamp` (ROP end), stored in a per-slot 4 KiB block.
  It is read in `begin_slot` after the existing slot fence: **no added waits or `glFinish`.**
- **Bounds and edge cases:**
  - 256 marks a slot, and a begin is taken only if its end and all open ends still fit;
  - 64 names, then "(other)";
  - list splits and resets need no special handling (same slot memory);
  - a pass left open across a present, or ended in another frame, is dropped and counted as
    unbalanced;
  - unmatched ends are ignored;
  - invalid or unwritten reports (0xFF…, reversed, >1 s) are rejected and counted.
- **Report every N frames:**
  - top-level ms/frame;
  - per pass: ms/frame, calls/frame and nesting depth (top 24);
  - dropped, rejected and unbalanced counts.
- **Limitations:**
  - nested passes are inclusive;
  - boundaries blur where the pipe overlaps passes;
  - work outside any `kGpu` section (e.g. the frontend's final upscale draw) is unattributed:
    compare top-level ms with the pacing line's scene span;
  - the present blit runs on the 2D engine and may finish after its mark;
  - the hooks assume `kGpu` sections run on the GL thread (true today: they wrap GL work).

### 2.5 Pacing counts, `OPENRAC_PACING_COUNTS=N` (30..3600 ticks; off by default)

- **Counts per N ticks:**
  - tick frames drawn;
  - left out late (`leave_late_frame`);
  - other ticks undrawn;
  - frames between ticks;
  - presents a tick;
  - ticks behind their time, and how many were over 100 ms behind (vsync's restart).
- **How:** `vsync.cpp` passes `behind` (lateness without the 100 ms cut-off) to `set_tick`, as a
  new defaulted 4th parameter. The existing `late` value and `leave_late_frame` are unchanged.

### 2.6 Experiment `OPENRAC_NO_BETWEEN_WHEN_BEHIND=1` (off by default)

- **What it does:** `between()` draws nothing while the current tick came ≥ half a tick late,
  measured without the 100 ms cut-off.
- **Why:** a between-tick frame is a full GPU frame that does not advance the game.
- **Expected if section 1.4 holds:** more ticks per second (game speed) at a similar present rate.
- **Trade-off:** less interpolation while slow.

## 3. Files changed

Back-ups of the previous versions are in
`build/claude-scratch/backup-pre-claude-pacing-pass-2026-10-30/`.

| File | Change |
|---|---|
| `port/renderer/dkgl/dkgl.h` | copy/full counters; `pass_begin` / `pass_end` / `pass_timing_enabled` |
| `port/renderer/dkgl/dkgl_internal.h` | `CopySource`; `before_transfer` returns bool; pass storage; gap / scene / tail samples |
| `port/renderer/dkgl/dkgl_core.cpp` | gap and scene/tail timing; pass read and report; source counters; late acquire; present blit pass |
| `port/renderer/dkgl/dkgl_gl.cpp` | copy sources at upload, mipmap, texture copy, blits; busy-upload naming; readback/mipmap Full counts |
| `port/common/stats.h`, `stats.cpp` | `PassHooks`, `set_pass_hooks`, Scope forwarding |
| `port/platform/switch/graphics.cpp` | installs the hooks when `OPENRAC_DK_PASS_TIMING` is set |
| `port/game/common/frontend.h` | `set_tick` gains defaulted `behind` |
| `port/game/common/frontend/state.h` | `Pacing::behind`; `PacingCounts`; `count_tick_frame` |
| `port/game/common/frontend/interpolation.cpp` | pacing counts and report; between experiment |
| `port/game/common/frontend/frame.cpp` | counts tick frames drawn or left out |
| `port/game/common/program/vsync.cpp` | passes `behind` |
| `port/tests/stats_pass_hooks_test.cpp` | **new** unit test |
| `build/deko3d-debug-reference/transfer-sync-mocktest.cpp` | acquire order, pass timing, pacing line and copy-source checks |

## 4. Tests (host only)

Real `dkgl_gl.cpp` + `dkgl_core.cpp` against the mocked deko3d, each mode in a clean process:

| Environment | PASS | FAIL |
|---|---|---|
| default | 29 | 0 |
| `LATE_ACQUIRE=1` | 29 | 0 |
| `PASS_TIMING=30` | 33 | 0 |
| `PASS_TIMING=30` + `LATE_ACQUIRE=1` + `DK_TIMING=30` | 35 | 0 |
| `DK_TIMING=120` | 31 | 0 |
| `DK_TIMING=30` | 31 | 0 |
| `DK_TIMING=30` + `LATE_ACQUIRE=1` | 31 | 0 |
| `TEXTURE_BIND_RUNS=0` | 29 | 0 |
| `SAMPLER_CACHE=0` | 24 | 0 |
| `LAZY_TARGET_BARRIERS=1` | 34 | 0 |
| `VERTEX_CACHE=0` | 22 | 0 |
| `PANEL_TRACE=1` | 30 | 0 |
| `SAMPLER_REBIND=first` | 29 | 0 |
| `SAMPLER_REBIND=new` | 29 | 0 |
| `DK_COUNTERS=30` | 29 | 0 |
| `SAMPLER_REBIND=0` | – | 1, pre-existing and expected |
| `TRANSFER_SYNC=legacy` | – | 3, pre-existing and expected |

New mock checks:
- the default acquire comes before the single submit;
- late acquire records submit → acquire → blit → submit → present;
- pass timing reports nested passes (depth 0/1) and the present blit;
- 300 marks in one frame stay bounded, with drops counted and every recorded begin paired;
- a pass left open across a present is counted as unbalanced;
- the pacing line and scene split are produced;
- the palette re-upload counts as a busy upload with its Full barriers by cause.

Other tests:
- `stats_pass_hooks_test` PASS: forwarding, nesting, CPU-only sections ignored, removal, works
  with stats disabled.
- `-Wall -Wextra -fsyntax-only` clean for `interpolation.cpp`, `frame.cpp`, `vsync.cpp` and
  `stats.cpp`, against current staged sources plus an older tree for unrelated headers.

**Limitations:**
- No Switch compile or link.
- `graphics.cpp` was not compiled here; it needs libnx and volk.
- The mock writes timestamps at record time, so ordering is tested, not GPU timing.
- Pacing counts and the between experiment were only syntax-checked.

## 5. Unresolved

- **Late acquire** is a hypothesis-driven fix. It depends on the gap metric showing an idle gap
  on hardware.
- The **between-frame mechanism** is inferred; the pacing counts will confirm or refute it.
- The **`first` sampler mode** stays opt-in. The short session-32 sample is not exhaustive.
- **Carried over:** cutscene audio delay, Pyrocitor preview material, and canopy attachment
  (activation only confirmed).

## 6. Hardware plan after Codex builds (one change at a time)

Conditions for every run:
- stock clocks;
- explicit 640x480;
- the same cap and route (Metropolis rotation, then flight and landing);
- `FRAME_COST` absent, lazy barriers 0, `SAMPLER_REBIND=1` (the default);
- add to the session-31 env: `OPENRAC_DK_PASS_TIMING=120`, `OPENRAC_PACING_COUNTS=120`. Keep
  `DK_TIMING`/`DK_COUNTERS`/`GAME_TIMING`/`SCENE_PASS_COUNTS` at 120.

Runs:
1. **Baseline with the new measurements.** Read:
   - the `GPU gap` versus present − span;
   - the per-pass table;
   - copy sources and busy-upload names;
   - the pacing counts (presents a tick, between frames, ticks over 100 ms behind).
2. **`OPENRAC_DK_LATE_ACQUIRE=1`.** Compare the present interval and FPS (Status Monitor), the
   gap/tail split and frame-time evenness. Look for any new tearing, flicker or a stale final
   picture.
3. **`OPENRAC_NO_BETWEEN_WHEN_BEHIND=1`** (late acquire back to default). Compare ticks/s
   (`OPENRAC_FPS`), presents a tick and game-speed feel.
4. If both help, combine them; then optionally `SAMPLER_REBIND=first`.
