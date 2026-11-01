# Claude review of the optimization pass (2026-10-29): source-only, not built

This reviews Codex's optimization pass from `HANDOFF_CLAUDE_OPTIMIZATION_2026-10-29.md` and the
canopy candidate from `DKGL_SHIP_CANOPY_SOURCE_FIX_2026-10-29.md`.

**Nothing here was built.**
- No game build, no shader-pack regeneration, no git commit and no push.
- The existing NROs and ELFs do not contain any of this.
- Every result below is a host test: real source compiled on a PC against a mock deko3d. None of it
  is a GPU or hardware result.
- Earlier hardware runs used overclocking. Nothing here claims a stock-clock gain or a confirmed
  visual fix.

Kept unchanged:
- the UAM shader fixes (sched init, no dual issue, stall before a new block) and the shipped pack;
- the SDK 0.5 workarounds (texture-cache invalidate mapping, sampler-set rebind, `BindVtxBuffers`
  capture/replay);
- the Full copy-engine barriers;
- saves;
- the launcher.

## Summary

| Item | Verdict | Change |
|---|---|---|
| Per-unit sampler descriptor cache | Correct | none |
| Sampler reflection layout cache | Correct | none |
| Palette clear reduction | Equivalent output | none |
| Lazy target barriers (OFF by default) | Sound at today's call sites, but fragile | **Hardened** (still OFF by default) |
| Game wall-interval timing (`OPENRAC_GAME_TIMING`) | Correct; opt-in | none |
| #126 language persistence | Matches upstream's code | none |
| Flight canopy candidate | **Never ran**: dead code | **Fixed** so it applies, plus an A/B switch and a one-time log line |
| MIP_AUDIT / DRAW_TRACE now default 0 | Right for performance | none |

## 1. Sampler descriptor cache (`dkgl_gl.cpp` `unit_handle`)

The cache key is the sampler parameters plus `image->levels`, and its value is the sampler
descriptor index.

**Descriptor lifetime:**
- `sampler_descriptor(params, levels)` is a pure function of its inputs. It normalises the min
  filter when there is one level, then searches the append-only `g->sampler_cache`.
- Descriptors are never freed or recycled. An index stays valid, with the same contents, for the
  whole context.
- So a cached index can never point at a reused slot.

**Invalidation:**
- Rebinding a different texture with the same parameters and level count correctly reuses the
  sampler. The image descriptor is still taken from `image->descriptor` on every draw.
- A change to wrap, filter or level count misses the cache. The mock checks both.

**Exhaustion:** when all 2048 descriptors are used, the function returns 0, and the cache stores 0.
That is the same answer the search would give again, because nothing is ever freed.

**Framebuffer feedback:** unaffected. Feedback detection, `touch` and `sampled_epoch` still run on
every draw; only the descriptor lookup is skipped.

## 2. Sampler reflection layout cache (`prepare_draw`)

- `Program::sampler_uniforms` is rebuilt when `sampler_layout_revision != link_revision` or when
  the uniform count changes.
- `load_program` increments `link_revision`, so a relink, or a reused program name, rebuilds it.
- The mock checks both cases: an unchanged shader keeps the layout, and a relink rebuilds it.

## 3. Palette memory clear (`scene/level_renderer.cpp` `set_palette`)

- The new code resizes, copies, then zero-fills only the padding past the copied data.
- The buffer contents after the call are byte-identical to the old full clear plus copy.
- `OPENRAC_PALETTE_COPY_REFERENCE` (any value) restores the old full clear for A/B testing.

## 4. Lazy target barriers: hardened

`OPENRAC_DK_LAZY_TARGET_BARRIERS=1` turns this on. It remains **OFF by default**.

### What Codex's version does

When the bound render target changes, the barrier is skipped unless one of these holds:
- `need_image_barrier` is set; or
- the old colour or depth target has `draw_write_epoch == barrier_epoch`, meaning it was written
  since the last barrier.

### Why it is safe today

- Every `barrier()` call waits; none is recorded with `DkBarrier_None`.
- The one barrier that does not invalidate the texture cache is `before_transfer`'s `(Full, 0)`.
  A copy-engine transfer always follows it. `after_transfer` then sets `pending_transfer` and
  `need_image_barrier`, and the next draw's `transfer_fence` adds `Full + Image` before anything
  samples.
- Write-after-read on the *new* target is covered indirectly. Any draw that sampled it also stamped
  the target that draw wrote, which is the old target here, so the old target's stamp forces the
  barrier.

### Why it was fragile

Two assumptions were implicit:
- **Every epoch bump also invalidates the texture cache before the next sample.** A future
  `barrier(x, 0)` that is not followed by a transfer fence would let a target written before it be
  sampled through a stale texture cache, with the switch skipped.
- **Every sampling draw stamps the target it writes.** The write-after-read safety depends on this.

### The hardening

- **`Context::ordered_epoch`** (`dkgl_internal.h`): a counter `barrier()` increments only when the
  barrier waits (mode is not None) *and* invalidates the texture cache (Image).
- **`draw_write_epoch`** is now stamped with `ordered_epoch`, in `prepare_draw` and in `clear`. The
  lazy check compares against `ordered_epoch`. A non-invalidating barrier therefore can never cause a
  skip.
- **Explicit write-after-read check:** the barrier is kept when the *new* colour or depth target has
  `sampled_epoch == barrier_epoch`, meaning it was sampled since the last waiting barrier.
- **Cost:** no extra barriers at today's call sites. `transfer_fence` and `image_barrier` always
  follow a non-invalidating barrier before a draw. A sample in the current epoch already implies a
  fresh stamp on the bound target.
- With lazy barriers OFF, the behaviour is unchanged. The new epoch only changes what the lazy check
  reads.

### Mock tests added (`transfer-sync-mocktest.cpp`)

1. A draw, then `barrier(Full, 0)`, then a target switch: the barrier is **kept**.
2. Old writes ordered by `Full + Image`, but the new target sampled in the current epoch: the
   barrier is **kept**.
3. Old writes ordered by `Primitives + Image`, and the new target not sampled since: the barrier is
   **skipped**.

Against Codex's original code, tests 1 and 2 **fail**. Against the hardened code, all pass. These
are synthetic hazards, which the current call sites do not reach.

## 5. Game wall-interval timing (`game/common/program/vsync.cpp`)

`OPENRAC_GAME_TIMING=N` (clamped to 30..3600) measures the time from `vsync()` returning to the
next `vsync()` call. It logs mean, p95 and max every N ticks.

- The constructor runs before the Switch exit-pad poll; the destructor stamps the time on return.
- The interval is the guest's own time between host vsyncs. As its log line says, it includes
  game-side I/O and waits. It is not a CPU profile.
- The first interval after a level load includes the load: expect a large max and use p95.
- It is off unless the variable is set, and the off path costs nothing measurable.

No change.

## 6. #126 language persistence (`game/rac1/host/options.c`)

Upstream adds `openrac_game_language = language;` after `openrac_game_set_option(...)`. The fork's
line is identical; only the comment wording differs. The rest of #126 is `mod_ui.cpp`, which the
fork does not have.

## 7. Flight canopy candidate: it never ran, now fixed

### The bug

- `render_scene` called `world_pose_alpha(snapshot.level, alpha)`, which forces alpha 1 only when
  that value is `-2`.
- `Kept::level` is set from `g->loaded` in `keep_tick`.
- `use_level(-2)` stores the **extracted flight world's number, 98** (`kFlightWorld`) in
  `g->loaded`, never -2.
- If the flight world is not extracted, it falls back to the title world (99) or 0. In `State`,
  `-2` means "nothing loaded".
- So the condition was never true, and the candidate was a no-op. The host test passed only because
  it calls `world_pose_alpha(-2, ...)` directly.

### The fix

- **`Kept::world`** (`state.h`): a new field, set in `keep_tick` (`frame.cpp`) from
  `world_to_draw(ram, openrac_game_loaded_overlay())`. That is the same function `use_world` uses,
  so it is -2 during the flight between planets: boot-program game mode 6, and ship state 4 inside a
  planet's program.
- **`render_scene`** now passes `snapshot.world`.
- **`geometry.h`:** the comment now states which value the function expects.
- **`OPENRAC_FLIGHT_POSE_AT_TICK=0`** turns the candidate off, for an A/B test in one build. It is
  on by default.
- **A one-time log line:** "frontend: flight between planets drawn at the game tick ..." appears
  when a flight frame is first drawn. A hardware log can then show the candidate actually ran.

### The trade-off to watch on hardware

The whole flight scene (camera, ship, joints, capsule shadows) now moves at the game's tick rate.
Frames drawn between ticks repeat the tick's pose. Flight motion may look less smooth than on
planets. This is the intended trade, not a regression, if the canopy stays attached. If the canopy
still detaches on frames drawn exactly at a tick, the cause is elsewhere. Possible causes are
effect-capture timing and the cockpit joint matrix, as the canopy doc says.

Host checks:
- `frontend_geometry_test` passes.
- `scene_frame.cpp` and `frame.cpp` compile with `-fsyntax-only`, against current staged sources
  plus an older full tree for unrelated headers (SDL3 headers from upstream 3.2.24). That checks
  syntax only, not a link.

## 8. Upstream menu and input fixes: assessment, nothing ported

- **#121 (980c98cb):**
  - Its "black loading screens" part is **already in the fork**. `scene_frame.cpp` uses
    `world_clear_colour(has_camera, draws_world, background)` with `draws_world = (draws & 0x1F)`,
    which matches upstream's `kWorldLayers` logic.
  - The NTSC parts do not affect the PAL Switch build: `confirm_prompt_input.c` and the
    `rac1_version.h` address fix.
  - The Exit row, its text IDs (0x7D00 to 0x7D80) and the main-menu row flight all live in
    `exit_menu.cpp` and `mod_ui.cpp`, which the fork lacks. The fork's `options_menu.cpp` has no
    Exit row and no 0x7Dxx text range, so the ID collision cannot happen here.
  - Not applicable.
- **#124 (0a9b904c):** `mod_ui.cpp` only. Not applicable.
- **#126 (5d778574):** already in the fork (section 6).
- **#122 (93ac5584): fresh input and low-latency frames.** Worth adapting later, carefully:
  - **`fresh_input`** re-reads the pads at the end of `vsync()`, just before the tick runs. On
    Switch, `Input::update()` runs in `read_window_events` at frame start, so the tick can see pad
    state up to one tick old when frames are drawn between ticks.
  - **What a Switch port needs:**
    - an `Input::refresh_pads()` for the SDL2 path (`SDL_GameControllerUpdate`, or libnx
      `padUpdate`, which `vsync` already uses for the exit combo);
    - a setting with an options row;
    - motors left in `update()`.
  - **Defaults:** keep it OFF, as upstream does, because it shifts input timing by about a frame.
  - **`low_latency_frames`:**
    - The `lag_ticks()` alpha change is platform-agnostic.
    - The swap-side part ("OpenGL waits for the card after each swap") would need a dkgl
      present-fence wait.
    - It is lower priority than measuring first.
  - Not ported in this pass. It needs a build to test, and it changes feel.
- **#119 (c6adc0d7) enhanced rendering:** not merged, as instructed.

## 9. Found while reviewing (not changed)

1. **Older hardware timing used `OPENRAC_FRAME_COST=120`** (`build/deko3d-test/env.txt`).
   - That mode calls `glFinish()` after every frame (`scene_frame.cpp`, `measure_cost`), which shows
     up as `flushes 1.00` in the dk counters. It serializes CPU and GPU.
   - Frame times from those runs (e.g. `transfer-sync-result`) are **not comparable** with a
     passive-timing run.
   - `env-passive-timing.txt` and `deko3d-release/env-performance-timing.txt` correctly leave it
     out.
2. **A latent ring-overflow hazard inside a draw, which predates this pass.**
   - `ring()` calls `flush_and_wait()` when the 48 MiB per-slot ring is full. That clears the
     command buffer and resets bound state.
   - If it happens inside `prepare_draw` after `bind_target`, or in `resolve()` after
     `prepare_draw` (`draw_indexed`, or CPU-skinned draws), the draw is recorded into a fresh buffer
     without its targets, shaders or descriptor sets. CPU-skinned vertices that were placed in the
     ring before the reset could also be overwritten.
   - Logged ring use is 44–845 KiB a frame, so this is not reachable in practice today.
   - **Recommended fix, if ring use ever grows:** detect a `ring_epoch` change across a draw's setup
     and redo the setup once.
3. **`Image::sampled_epoch` had no reader before this pass.** It now feeds the write-after-read
   check.

## Test results (host, Linux g++ 13.3, against a mock deko3d)

`transfer-sync-mocktest.cpp` compiles the real `dkgl_gl.cpp` and `dkgl_core.cpp`:

| Environment | PASS | FAIL |
|---|---|---|
| default | 22 | 0 |
| `OPENRAC_DK_SAMPLER_CACHE=0` | 17 | 0 |
| `OPENRAC_DK_LAZY_TARGET_BARRIERS=1` | 27 | 0 (5 lazy checks, 3 new) |
| `OPENRAC_DK_LAZY_TARGET_BARRIERS=1 OPENRAC_DK_SAMPLER_CACHE=0` | 22 | 0 |
| `OPENRAC_DK_TIMING=120` | 23 | 0 |
| `OPENRAC_DK_VERTEX_CACHE=0` | 15 | 0 |
| `OPENRAC_DK_TRANSFER_SYNC=legacy` | – | 3 expected: legacy has no Full barriers by design. Codex's original code fails the same 3. |
| Codex's original dkgl, `LAZY=1`, with the new tests | – | 2 (the hardening tests) |

Other host tests:
- `frontend_geometry_test`: PASS.
- `timing_samples_test`: PASS.
- The dkgl unit tests in `port/tests` all PASS: `diagnostics` (shader sources from an older tree
  snapshot), `draw_state`, `geometry_validation`, `sampler_refresh`, `transfer_validation` and
  `vertex_binding`.

## Files changed in this pass

Back-ups of the previous versions are in
`build/claude-scratch/backup-pre-claude-optimization-review-2026-10-29/`.

- `port/renderer/dkgl/dkgl_internal.h`: `ordered_epoch`, plus `draw_write_epoch` documented.
- `port/renderer/dkgl/dkgl_core.cpp`: `barrier()` increments `ordered_epoch`.
- `port/renderer/dkgl/dkgl_gl.cpp`: the lazy check uses `ordered_epoch`, adds the
  write-after-read check, and the stamps use `ordered_epoch`.
- `port/game/common/frontend/state.h`: `Kept::world`.
- `port/game/common/frontend/frame.cpp`: `keep_tick` sets `k.world`.
- `port/game/common/frontend/scene_frame.cpp`: pose alpha from `snapshot.world`, the A/B switch and
  the one-time log line.
- `port/game/common/frontend/geometry.h`: comment only.
- `build/deko3d-debug-reference/transfer-sync-mocktest.cpp`: three new lazy-barrier checks.

## Recommended next steps (only after you give permission to build)

1. **Build once with everything at its default.** Lazy barriers stay off; the sampler cache, the
   layout cache, the palette change and the canopy fix are on. Record the ELF hash.
2. **Measure at stock clocks** with `env-passive-timing.txt`, with no `OPENRAC_FRAME_COST`.
   - Use the same route each time; 480p and 720p, docked and handheld.
   - Compare `dk gpu`/`present`/`cpu timing` against a baseline from the pre-optimization ELF (the
     `before-stock-clock-optimization` diagnostics), at the same clocks.
3. **Change one switch at a time:**
   1. `OPENRAC_DK_SAMPLER_CACHE=0` against the default;
   2. `OPENRAC_PALETTE_COPY_REFERENCE=1` against the default;
   3. `OPENRAC_DK_LAZY_TARGET_BARRIERS=1` against the default. Watch for any flicker, a stale
      reflection or shadow, or a menu panel showing old contents. Check `target_barriers_skipped`.
   4. `OPENRAC_GAME_TIMING=120`, to split guest time from host time.
4. **Canopy:**
   - Fly close to the ship, then land.
   - Confirm the "flight between planets drawn at the game tick" log line appears.
   - Compare against `OPENRAC_FLIGHT_POSE_AT_TICK=0` in the same build.
   - Report both canopy attachment and flight smoothness.
5. **Only then** consider #122's `fresh_input` for Switch, behind an off-by-default setting.
