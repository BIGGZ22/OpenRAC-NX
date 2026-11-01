# Claude review: session 33, scenery cost, pacing and test logs (2026-10-30). Source only, NOT BUILT

**Nothing here was built or tested on hardware.**
- Claude did not build the game, regenerate shader packs, commit or push.
- The session-33 NRO/ELF pair does not contain these changes.
- All results are log analysis, host compiles and host tests.

## 1. What session 33 says (build/deko3d-test/pacing-baseline-review/logs)

Setup: 640x480 internal, stock clocks, `SAMPLER_REBIND=1`, lazy barriers off, late acquire off,
no-between off.

**Heavy window: per-pass GPU timestamps (elapsed, not pure busy time)**

| Pass | ms/frame |
|---|---|
| Ties translucent | 17.8 |
| Ties solid | 14.9 |
| Ties cutout | 7.0 |
| Terrain | 10.9 |
| Shrubs | 4.1 |
| Mobys opaque | 4.0 |
| Everything else | < 0.5 each |

- Top-level passes: 60.7 ms/frame. Scene span: 61.2 ms. GPU gap: 5.3 ms. Present interval:
  67.0 ms.
- Render CPU record is 16.5 ms/frame and the game is 13.9 ms/tick. Both are well under the GPU.

### Ties: the translucent pass is not camera-culled per instance

`draw_layer`: `culled = mobys ? !metal : pass <= 1`.
- For the static layers, the solid and cutout passes draw only the instances the camera culling
  kept (`camera_count`, compacted buffer).
- The translucent pass drew every PVS-visible instance of a visible group (`pvs_count`).
- A tie group is every instance of a tie model across the level. Its box is the union, so the
  group is nearly always "visible".
- So every PVS-visible instance behind or beside the camera had its translucent-material vertices
  transformed and clipped.

Session 33 fits this:
- the cutout pass (same cutout-material geometry, culled) costs 7.0 ms;
- the translucent pass (unculled, blended) costs 17.8 ms;
- the difference also includes blending and different fragment work, so the saving is not proven.

### Scenery detail

- The port's default "Scenery detail: Full detail" draws every tie and shrub however far away
  (`settings.h`: `original_lod = false`).
- "Original draw distances" applies the PS2 game's own distances (`LevelScene::choose_scenery`).
- Session 33's log does not say which setting was in use. This pass logs it.

### Pacing

- Windows with 41–117 between-tick frames per 120 ticks have every tick behind, no >100 ms
  restarts and no frames left out.
- `between()` predicts when a frame will be shown from `Pacing::cost`, the window thread's CPU
  time before the swap. That excludes GPU time. At ~45–67 ms per present, it still predicts a
  refresh before the next tick, so whole GPU frames are drawn that do not advance the game.
- Late ticks are never left out, even though `leave_late_frame` exists. One possible reason:
  ticks whose 2D list uploads an image (`sends_image`) are always drawn. This pass counts the
  reasons.

### Copies

- About 1.7–2 uploads and about 2 Full barriers per frame, mostly the transfer fence.
- Framebuffer and mipmap copies: 0.
- The busy-upload names were used up at startup.

## 2. Changes (file by file)

### 2.1 Translucent camera culling for static layers (DEFAULT ON, reference switch)

`port/scene/level_renderer.cpp`, `draw_layer`:
- `culled` now also covers `pass == 2` for ties, shrubs and terrain.
- **`OPENRAC_STATIC_TRANSLUCENT_CULL_OFF=1`** (a `stats::probe` like the other references)
  restores the old submission.
- **Why it should be pixel-identical:**
  - The same per-instance boxes (`instance_bounds`: the model AABB through the instance matrix,
    padded) and the same six planes as the solid and cutout passes.
  - An instance wholly outside the frustum yields no fragment.
  - The compacted survivors keep their original order, so blending order is unchanged.
  - Instances without a valid box are kept.
  - No shader or pack change.
- **Risk:** a vertex shader that moved static geometry outside its model box. None does today
  (static: no skinning; metal is moby-only).

### 2.2 Static layers in `OPENRAC_SCENE_PASS_COUNTS`

Same file. A new line for each layer: `ties|shrubs|terrain pass snapshot frame N`. It gives:
- per pass (solid/cutout/translucent): draws, instanced triangles, groups and instances;
- for the layer: instance total, group count, camera-kept, unbounded;
- whether the translucent pass is culled.

The moby line is unchanged.

### 2.3 Scenery choice logged

`choose_scenery`: when "Original draw distances" changes what is drawn, the first 24 changes per
run are logged as `scenery (original draw distances): ties X of Y drawn`.

### 2.4 Graphics settings logged

`port/game/common/frontend/scene_frame.cpp`: `graphics settings in use: …` at the first frame and
on every change. It records:
- scenery detail;
- texture filter, anisotropy and samples;
- render scale and Switch resolution preset;
- vsync, frame-rate mode and cap.

### 2.5 Experiment: `OPENRAC_STATIC_FRONT_TO_BACK=1` (off by default)

`level_renderer.cpp/.h`:
- The static opaque passes (solid, cutout) draw groups ordered by their nearest kept instance
  (`Group::nearest`, set during camera culling), via `m_group_order`.
- Instances keep their order.
- **Aim:** more early-depth rejection in the tie solid pass. The solid shader has no discard.
- **Opt-in because** faces at exactly equal depth may resolve the other way (`GEQUAL`: the later
  draw wins).

### 2.6 Experiment: `OPENRAC_BETWEEN_PRESENT_COST=1` (off by default)

`port/game/common/frontend/{interpolation.cpp,state.h}`:
- **`Pacing::present_period`:** a smoothed swap-to-swap interval (each sample ≤ 250 ms). It
  holds GPU time when the GPU is the slower, because dkgl's swap waits on the previous frame's
  fence.
- **The rule:** `between()` declines a frame when `now + present_period >= next_tick + lag`.
- **Expected:** presents per tick → about 1 in GPU-bound views, and ticks per second up (game
  speed, animation/audio sync). Interpolation remains when the display has spare time.
- **Compared with `OPENRAC_NO_BETWEEN_WHEN_BEHIND=1`:** that experiment declines only when the
  tick is ≥ half a tick late. This one also covers ticks that are only slightly late.

### 2.7 Pacing counts extended

The `pacing over N ticks` line now also gives:
- frames declined by each experiment;
- late ticks still drawn, by reason: three already left out, the 2D list uploads an image, or
  other;
- display pace and CPU cost.

### 2.8 Upload shapes (dkgl, `OPENRAC_DK_COUNTERS`)

`dkgl_internal.h/gl.cpp/core.cpp`:
- **New line:** `dk upload shapes over N frames (K shapes): WxH format F level L rect wxh: count
  (busy b); …`, the 6 most frequent per counters window.
- At most 256 shapes per window.
- This replaces the need for the startup-exhausted first-8 names (those remain).

### 2.9 Test logs on the SD card (user request)

New `port/game/common/test_logs.h` (header-only); `port/game/common/main.cpp` calls it on the
Switch before the run's log is opened.

**Where the logs go:**
- **Each run:** `sdmc:/switch/openrac/test-logs/<YYYY-MM-DD_HH-MM-SS>/`.
- **Newest ended run:** `test-logs/latest/` and `LATEST.txt`.

**What is copied:**
- the game log (`data/openrac-rac1-pal.log`);
- `env-applied.log`, `deko3d-startup.log` and `vulkan-startup.log`;
- only this run's part of the growing logs: `overlay-status.log`, `dkgl-driver.log`,
  `deko3d-runtime.log` and `vulkan-runtime.log`.

**When:**
- At the program's exit: HOME-menu close, L+R+Minus or the game's quit, i.e. `std::exit` →
  `atexit`. The folder gets `COMPLETE`.
- A run without a clean exit (crash, power) is copied when the next run starts, with `NOTE.txt`.

**Housekeeping:**
- the 20 newest run folders are kept;
- `OPENRAC_TEST_LOGS=0` disables it.

## 3. Not changed, and why

- **Shaders and packs:** they cannot be regenerated, so the translucent pass's texture fetch
  before its discard stays.
- **Translucent pass skipping by texture alpha:** not done. Bilinear filtering makes partial
  alpha at every 0/1 edge, and lit ties take alpha from the light texture.
- **Per-primitive terrain frustum culling:** a candidate. Terrain (10.9 ms) gets PVS primitive
  culling but no frustum test per fragment primitive. It needs per-primitive bounds (computable
  at upload from `level.vertices`) and breaks material runs into more draws. It deserves its own
  pass after the pass counts show terrain's share.
- **Tie LODs:** the port draws every tie at its most detailed LOD. The PS2 used lower LODs far
  away. Only the top LOD appears to be extracted.
- **Leaving late frames out when the 2D list sends an image:** that would need the image upload
  without the 3D frame. Wait for the late-kept-reason counts.
- **Upload path:** keep the copy engine and Full ordering until the shapes name the uploads.

## 4. Tests (host)

**Mock dkgl** (the real `dkgl_gl.cpp` and `dkgl_core.cpp`), one clean process per mode, 0
failures in each:

| Mode | PASS |
|---|---|
| default | 29 |
| `DK_COUNTERS=1` (new upload-shape check) | 30 |
| `DK_COUNTERS=30` | 30 |
| `LATE_ACQUIRE=1` | 29 |
| `PASS_TIMING=30` | 33 |
| `DK_TIMING=30` + `LATE_ACQUIRE=1` | 31 |
| `DK_TIMING=120` | 31 |
| `TEXTURE_BIND_RUNS=0` | 29 |
| `SAMPLER_CACHE=0` | 24 |
| `LAZY_TARGET_BARRIERS=1` | 34 |
| `VERTEX_CACHE=0` | 22 |
| `PANEL_TRACE=1` | 30 |
| `SAMPLER_REBIND=first` | 29 |
| `SAMPLER_REBIND=new` | 29 |

Expected, pre-existing failures: `SAMPLER_REBIND=0` → 1; `TRANSFER_SYNC=legacy` → 3.

**Other host tests:**
- **`test_logs_test` (new): PASS.** Run in an empty folder, it covers:
  - the per-run folder and growing logs' own part;
  - `COMPLETE` and `latest`/`LATEST.txt`;
  - crash recovery with `NOTE.txt`;
  - the same-second name `_2`;
  - pruning to 20.
- **`-Wall -Wextra -fsyntax-only` clean:**
  - `level_renderer.cpp`, `interpolation.cpp`, `frame.cpp`, `scene_frame.cpp`;
  - `main.cpp`, compiled both with and without the Switch-only call.

**Not proven:**
- pixel equivalence of the translucent cull on hardware (argued, not imaged);
- any FPS change;
- the pacing experiments' behaviour (only syntax-checked: they depend on live timing);
- front-to-back's effect;
- on-device file I/O of `test_logs.h` (`fs::copy_file`, `remove_all` on sdmc:). It uses only
  standard calls, but they are untested on libnx.

## 5. Hardware plan (after Codex builds)

**Baseline conditions for every run:**
- stock clocks; 640x480; the same cap and route;
- the session-33 env, i.e. `DK_PASS_TIMING`/`PACING_COUNTS`/`SCENE_PASS_COUNTS`/`DK_COUNTERS`
  at 120;
- no `FRAME_COST`; `SAMPLER_REBIND=1`; lazy 0.

**Runs:**
1. **New default build.** The translucent cull is on.
   - Compare `ties translucent` ms with session 33, and the new ties snapshot (instances by pass).
   - Note the `graphics settings in use` line.
   - Check the image: foliage edges, fences, glass (side by side with
     `OPENRAC_STATIC_TRANSLUCENT_CULL_OFF=1` at the same spot).
2. **`OPENRAC_BETWEEN_PRESENT_COST=1`:** presents per tick, ticks per second (`OPENRAC_FPS`),
   landing animation/audio sync, feel.
3. **`OPENRAC_DK_LATE_ACQUIRE=1`:** Codex's prepared config, separately.
4. **`OPENRAC_STATIC_FRONT_TO_BACK=1`:** ties solid/cutout ms; look for z-fighting changes.
5. **Launcher "Scenery detail: Original draw distances":** ties/shrubs ms and visible pop-in.
   This is a user-facing choice, not a silent default.

The logs for every run will be in `sdmc:/switch/openrac/test-logs/latest/` (and its dated
folder).
