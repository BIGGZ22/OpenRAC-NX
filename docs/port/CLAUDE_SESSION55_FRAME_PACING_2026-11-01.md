# Claude — session 55: diagnostic-off semantics and tick-frame pacing (2026-11-01)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.
`hardware_tested=false`.

What stays as it was:
- the tested NROs:
  - release `474c5ae5e380806e9df07315cbd7ae27c89bfba43c24cb97a5be7ebe4b84286f`, ELF `589ca9822e1685719ca591f0f4be171075d4b0d0`;
  - diagnostic `3d3b178fee156fa95acc3f1b1a0d7d3edff1b334568c45286d6521b66619fba6`, ELF `50c19de4a784cb85482bb5e27f580b7d6471c324`;
- their `last-build.elf/json`, every config and log, the staged UAM shader packs;
- the renderer work: the deko3d 0.5 workarounds, the transfer Full barriers, the vendor/Pyrocitor/fade fixes;
- the timing and engine work: the worker cap anchor (real frame start), the PAL 50/60 time base, the audio queue
  semantics and the pose scratch reuse.

Backups: `build/claude-scratch/backup-pre-claude-session55-2026-11-01/` (every file changed, as it was).

## 1. Priority 1: diagnostic "off"

**Established (source).**
- `dkgl_core.cpp` read `OPENRAC_DK_COUNTERS` as `v ? max(1, atoi(v)) : 0`, so `=0`, `=-1`, `=` and `=abc`
  all meant "every frame".
- `dkgl_gl.cpp` turned the upload-shape map and the busy-upload lines on whenever the variable existed.
- Session 55's clean config set `=0` and so logged dk counters every frame (log ~4.47 MB). It is **not**
  diagnostics-off evidence, and this does not show that logging caused the stutter.

**Change.**
- New `port/common/env_interval.h` (header only, `constexpr`, no allocation): `env_interval::parse(v, lo, hi)`.
  - Unset, empty, 0, negative or not a number: 0 (off).
  - A positive N is read as `atoi` reads it, then held to [lo, hi]. Overflow holds at `INT_MAX`.
- `dkgl_core.cpp`: `every = env_interval::parse(getenv("OPENRAC_DK_COUNTERS"))`.
- `dkgl_gl.cpp`: `counting = env_interval::on(...)`. An off value no longer fills the upload-shape map (the
  diagnostic's only allocation) and no longer names busy uploads. The `g->counters` increments stay: they are
  plain integer adds the renderer keeps anyway. Renderer synchronization is untouched.
- **Positive values behave exactly as before** (`120` gives every 120 frames).

**Audit of the other diagnostics (source).**

| Variable | Status |
|---|---|
| `OPENRAC_CPU_TIMING`, `DK_TIMING`, `DK_PASS_TIMING`, `GAME_TIMING`, `PACING_COUNTS`, `SCENE_PASS_COUNTS`, `TICK_PHASES` | Already `atoi > 0` (off for 0, negative and empty). Unchanged. |
| `POSE_VERIFY`, `DK_PANEL_TRACE`, `DK_MIP_AUDIT`, `DK_DRAW_TRACE` | Already `<= 0` off. Unchanged. |
| `OPENRAC_AUDIO_TRACE`, `MC_TRACE`, `DK_GEOMETRY_AUDIT`, `DK_LAZY_TARGET_BARRIERS`, `DK_LATE_ACQUIRE` and the reference switches | Already `== "1"`. Unchanged. |
| `OPENRAC_PALETTE_COPY_REFERENCE` (reference) | Was presence (`=0` turned it on). Now `"1"` only. |
| `OPENRAC_DK_DIAGNOSTIC=0` (not a counter) | Skips the dkgl self-test and is then rejected as unknown. Left alone; noted. |

The `stats::probe` flags follow `env.cpp`'s table, which defines each flag's truth explicitly.

## 2. Priority 2: tick frames held back and never drawn

### What the code does (established, source)

1. **Two separate schedules.**
   - The tick schedule (`vsync.cpp`, `tick_pacing::next_due`) puts ticks exactly a period apart.
   - The frame cap's schedule (`interpolation.cpp swapped`, `frame_pacing::next_due`) is `previous due + step`,
     starting again from a frame's start only when a swap comes more than two steps after its due time (a hitch).
2. **When a tick's frame starts.** It starts after the tick's game work, the capture and the wait for the last
   render job (`vsync()`: `prepare_frame`, `finish_render`, `set_tick`, then `draw_tick_frame`). That moment
   varies by several ms from tick to tick (session 54: CPU record 3.2–3.8 ms on average, up to 10.5–14.2 ms).
3. **Why tick frames were held back.**
   - The cap let a frame start only within **1 ms** of its due time (`frame_may_start`).
   - After a hitch the cap's schedule is counted from a late frame start.
   - From then on, every tick whose work finished more than 1 ms sooner than that one was held back, even at
     cap 60 / tick 60, where ticks already come no faster than the cap.
4. **Why held-back frames were lost.** A held-back frame (`first_pending`) is drawn only by `between()`, and
   `between()` first applies `OPENRAC_NO_BETWEEN_WHEN_BEHIND`.
   - That experiment returns at once when `behind > period/2`.
   - `behind` is measured at `set_tick`, after the tick's own work and the wait for the last frame. So any
     tick with more than ~8.3 ms of work counts as "behind" (session 54: "ticks behind their time 120" of 120;
     "declined behind 88").
   - **The tick's only picture was declined as if it were an extra between-frame. That is the "held back and
     never drawn".**
5. **A second effect of the same early return.**
   - With the render worker, a tick's finished frame is normally swapped inside `between()`, once
     `ready_until(next_tick)` reports the job done.
   - On a "behind" tick the early return skipped that, so the swap waited for the next tick's game work
     (`finish_render` at the next `vsync()`).
   - So swap times alternated between "render done" and "next tick's work done". That is CPU present jitter,
     with vsync 0.
6. **Other ways a held-back frame was dropped silently** (no counter):
   - the next tick is due before the cap's due (correct when the cap is under the tick rate);
   - the worker is busy past the next tick;
   - the frame cannot finish before the next tick;
   - `draw_tick_frame` held a frame back where `between()` is never called (`!wants_between()`, e.g. no camera
     yet).

### Model evidence (host test, the real pacing functions)

`tests/pacing_model.h` writes out the order of `vsync()`, `draw_tick_frame`, `between`, `swapped` and the render
worker over a simulated clock. It calls the real `frame_pacing::next_due`, `frame_may_start`,
`tick_frame_may_start` and `tick_pacing::next_due`.

Session-54-like inputs: cap 60, ticks 60, vsync 0, worker on, between-frames declined, tick work 2.5–11 ms,
render job 3–5 ms (12 ms every 37th frame), and a 40 ms hitch every 150 ticks. Results over 3600 ticks:

| | Reference (old rules) | New |
|---|---|---|
| Tick frames held back | 1323–1367 (drawn late after a sleep, or lost) | 48 (the catch-up ticks right after a hitch) |
| Lost | 47–48, all "declined behind" | 48, all "next tick first": the catch-up tick's frame is replaced by the next one, which follows within ms |
| CPU swap-gap p95 | 31.7–32.0 ms | 23.2–23.5 ms |
| With 25 ms hitches: lost | 5–9 | 0–1 |
| Held back, 25 ms hitches | 2022–2067 | 1–184 |

- **The old rules reproduce what the session-54 log counted:**
  - frames held back and never drawn, all through the behind decline;
  - many held back and drawn late;
  - uneven swaps.
- **What remains in the new rules:**
  - frames that two ticks run back to back would show within a few ms of each other;
  - the swap jitter from tick work itself (vsync 0).

### What is not established

- **Which share of session 54's 3–4 per 120 came from each path.** The pacing line did not split the reasons.
  It now does.
- **What the eye sees.** The gaps are between CPU present calls (`dkQueuePresentImage` with vsync 0), not
  scan-out. No timing of the display's own flip is available here.
- **Session 55's "about 50 FPS" impression.** It came from the every-frame counter logging build, and the
  guest timing shows 50 → 60 at frames 222 → 1247. **Hypothesis:** the counters' per-frame SD logging slowed
  it. Run 1 below is the first genuinely clean run.

### Changes (default on; one reference switch restores all of them)

`OPENRAC_TICK_FRAME_PACING_REFERENCE=1` gives the pre-session-55 behaviour for (1)–(4).

1. **Tick-frame tolerance** (`pacing.h/.cpp`: `tick_frame_tolerance`, `tick_frame_may_start`; `state.h` /
   `interpolation.cpp`: `tick_frame_due`).
   - When the cap is at least the tick rate (step ≤ tick period + 0.25 ms, so 1/60 s against tick_pacing's
     16 666 µs counts), and no between-frame was drawn since the last tick frame (`pacing.between_since_tick`),
     a tick's frame may start up to **half a cap frame** before the cap's due time.
   - Otherwise the 1 ms as before: cap under the tick rate, or a between-frame took the slot.
   - The cap's schedule is unchanged (`previous + step`), so the rate over time stays at or under the cap.
   - Only a tick frame closer than half a cap frame to the last (a catch-up tick) is held back.
2. **The behind decline no longer drops a held-back tick frame.** `NO_BETWEEN_WHEN_BEHIND` still declines
   between-frames only.
3. **A finished render job is swapped before the behind decline.** Waiting in `ready_until(next_tick)` costs
   nothing: the game thread would sleep until `next_tick` anyway.
4. **No hold-back where nothing would draw it later** (`!wants_between()`).

Unchanged:
- the worker anchor (`output.started`);
- the tick schedule, the PAL 50/60 handling, the 100 ms catch-up slack;
- the swap interval and vsync, the caps;
- no `glFinish`, no forced 50.

### Diagnostics added

**Pacing line (`OPENRAC_PACING_COUNTS`)** has a new clause: "held back and never drawn, by why: next tick
first / render thread busy / too late / declined behind (reference); drawn at once by the tick tolerance N".

**`OPENRAC_PACING_RING=N`** (64..16384; off by `env_interval`):
- **How it records.** The last N events go into a fixed ring with one allocation, and are written to the log
  **once, at `frontend::close()`** (the L+R+Minus exit). Nothing is written to the SD card while playing.
- **What each event holds.**

  | Event | Fields |
  |---|---|
  | `tick` | rate, due time, behind |
  | `drawn` / `deferred` | the cap's due and the tolerance used |
  | `deferred-drawn` / `lost-*` | why |
  | `swap` | the frame's start, the swap call's time, CPU record and GPU time |

- **Limits.**
  - Swap = the CPU present call.
  - Acquire and slot-fence waits stay in `OPENRAC_DK_TIMING`'s aggregate; they are not per frame.
  - The `tick` field is the kept frame number when the event happened. At a `tick` event it can still be the
    previous tick's.

**`port/tools/pacing_ring_summary.py LOG`** prints:
- counts per 120 ticks;
- swap-gap, frame-start-gap, start-to-swap, record, GPU and behind distributions;
- the worst swap gaps with the events between them.

### Experiment (off by default, not in the two configs)

`OPENRAC_EVEN_PRESENT=1`:
- **What it does.** A tick's finished frame, swapped in `between()`, is swapped no sooner than one tick less
  1 ms after the last swap, and never past the next tick's time.
- **Model.** Swap-gap p95 23.7 → 20.8 ms, nothing lost.
- **Hardware.** For a later run, once runs 1 and 2 are read.

## 3. Priority 3: optimizations

None this pass, deliberately. With the clean data unavailable (session 55 was invalid), there is no measured
allocation, copy or log cost to cut. The diagnostic-off fix removes the proven one: per-frame counter logging
and the upload-shape map. Pose reuse and conservative uploads are untouched.

## 4. Files

| File | Change |
|---|---|
| `port/common/env_interval.h` | new |
| `port/renderer/dkgl/dkgl_core.cpp` | `DK_COUNTERS` via `env_interval::parse`; include |
| `port/renderer/dkgl/dkgl_gl.cpp` | upload counting via `env_interval::on`; include |
| `port/scene/level_renderer.cpp` | `PALETTE_COPY_REFERENCE` `"1"` only |
| `port/game/common/frontend/pacing.h`, `pacing.cpp` | `tick_frame_tolerance`, `tick_frame_may_start` |
| `port/game/common/frontend/interpolation.cpp` | reference switch, `EVEN_PRESENT`, ring, `tick_frame_due`, `between()` order and loss counters, swap ring event, pacing line |
| `port/game/common/frontend/frame.cpp` (CRLF) | `draw_tick_frame`: `tick_frame_due`, no hold-back without `between()`, counters, ring |
| `port/game/common/frontend/state.h` (CRLF) | `Pacing::between_since_tick`, `PacingCounts` loss reasons, declarations |
| `port/game/common/frontend.cpp` | `close()`: `dump_pacing_ring()` |
| `port/tests/env_interval_test.cpp` | new |
| `port/tests/pacing_model.h` | new (model) |
| `port/tests/frontend_pacing_test.cpp` | tolerance and model tests |
| `port/CMakeLists.txt` | `openrac_add_test(env_interval INCLUDES .)` (`frontend_pacing` is already in `cmake/Window.cmake` with `pacing.cpp`) |
| `port/tests/env_test.cpp`, `docs/port/DEBUGGING.md` | five rows: `TICK_FRAME_PACING_REFERENCE`, `EVEN_PRESENT`, `PACING_RING`, `DK_COUNTERS`, `PALETTE_COPY_REFERENCE`; `NO_BETWEEN_WHEN_BEHIND` and `PACING_COUNTS` rows updated |
| `port/tools/pacing_ring_summary.py` | new |
| `build/deko3d-release/session55-tests/{01-clean-reference,02-tick-frame-pacing}/env.txt` | new |

## 5. Tests (Linux container)

**`frontend_pacing_test`** (g++ `-Wshadow`, clang `-Wconversion`, g++ ASan/UBSan): exit 0. It covers:
- the tolerance table (60/60, 60/50, 120/60, 144/50, 30/60, 30/50, after a between-frame, no cap, the
  1 ms floor) and `tick_frame_may_start`;
- the session-54 model above (the old rules lose frames to the behind decline; the new lose none to it, hold
  back only catch-ups, have a lower swap p95 and the same game time);
- 25 ms hitches;
- cap 60 / ticks 60, worker on and off: every frame drawn, 60 a second, game time kept;
- cap 60 / ticks 50, worker on and off;
- cap 30 / ticks 60, reference and new, worker on and off: 1790–1801 frames in 60 s, ≤ 31 starts in any
  second, starts ≥ 15.7 ms apart;
- 50 → 60 at tick 500: every frame drawn, no burst, 10 s per segment;
- a 500 ms stop: ticks after it ≥ 16 ms apart, no burst, every frame drawn;
- delayed render jobs (20 ms every 10th): nothing lost;
- `EVEN_PRESENT`.

**Other checks:**
- `frame_pacing_test`: exit 0 (unchanged).
- `env_interval_test`: g++ and clang `-Wconversion`, ASan/UBSan, exit 0. It covers unset, empty, 0, `00`,
  negatives, non-numbers, `-0`, `atoi` agreement, overflow, bounds and the counters' schedule (0 lines over
  600 frames for every off value; 5 at 120).
- `env_test`: exit 0, with DEBUGGING.md's rows matching.
- `interpolation.cpp`, `frame.cpp`: `-fsyntax-only -Wall -Wextra -DOPENRAC_FRONTEND`, gcc and clang, clean.
  The only `-Wshadow` warnings are pre-existing (`report_pacing_counts`' `n`).
- `frontend.cpp`: clean apart from the missing `stb_image.h` in this container.

**Not compiled here:**
- `dkgl_core.cpp` and `dkgl_gl.cpp` (no deko3d headers; the change is one include and one line each);
- `level_renderer.cpp` (stale generated headers here; one line changed).

Also not covered: the AArch64 build, the Windows tests, hardware.

## 6. Hardware plan (NEW build; `build/deko3d-release/session55-tests/`)

Use the same route as session 54: handheld, Horizon OC as it is, game clock requests off. Play at least 2
minutes and **exit with L+R+Minus** (the ring is written at close). Each config sets the clean set plus only
`OPENRAC_PACING_RING=16384`.

| Run | Config | What it is |
|---|---|---|
| 1 | `01-clean-reference` | `OPENRAC_TICK_FRAME_PACING_REFERENCE=1`: old pacing, genuinely clean (no counter, timing or pacing-count logging) |
| 2 | `02-tick-frame-pacing` | the new default |

Read with `python3 port/tools/pacing_ring_summary.py "<log>"`. Compare:
- lost per 120 ticks, and by why;
- held back per 120;
- swap-gap p95 and max;
- the worst gaps' events;
- feel: the same spot, the same camera pans.

Check that the env line in the game's "environment in effect" output has no `DK_COUNTERS`.

If run 2 still feels uneven while it loses nothing, the remaining candidate is swap jitter from tick work.
Then run `OPENRAC_EVEN_PRESENT=1` on top of config 2.
