# Claude review: session 34 scenery cost, terrain batching, pass statistics, exit path (2026-10-30)

**CLAUDE DID NOT BUILD.**
- No game build, no shader-pack regeneration, no commit and no push.
- The tested performance NRO `0f114116…` (ELF `1ed1d0c7…`) and its ELF/manifest are untouched.
- Everything below is log analysis, host compiles and host tests.

## 1. Evidence: session 34 (`build/deko3d-test/scenery-baseline-review/logs`)

Settings (from the new graphics line): original draw distances, nearest filter, 1 sample, 640x480,
vsync 0, cap 60. Translucent camera culling is on.

**Heavy window**

| Pass | ms/frame |
|---|---|
| Ties solid | 15.0 |
| Terrain | 11.0 |
| Ties cutout | 6.9 |
| Ties translucent | 6.0 |
| Mobys opaque | 3.8 |
| Shrubs | 2.6 |

Scene span 47.7 ms; present interval 55.9 ms.

**Frame 3600 pass snapshots**

| Layer | Draws (solid / cutout / translucent) | Triangles (solid / cutout / translucent) | Instances drawn | Instances total | Camera-kept |
|---|---|---|---|---|---|
| Terrain | 1637 / 73 / 73 | 39,464 / 810 / 810 | 702 | 1006 (1006 groups) | 702 |
| Ties | 347 / 32 / 32 | 299,744 / 149,180 / 149,180 | 1739 | 5081 (127 groups) | 1739 |

**What it means:**
- **Terrain is draw-bound:** about 24 triangles a draw. Its 11 ms is consistent with a per-draw
  cost of roughly 6–7 µs on this GPU at 307 MHz (Codex's Vulkan notes found ~10 µs a draw on NVK).
- **Ties are triangle-heavy:** 300k solid triangles, plus 149k drawn twice (cutout and
  translucent).
- **`deko3d: baked camera visibility (PVS) off`:** the game's own PVS was not used on deko3d. It
  is on by default in the Vulkan backend. It was kept off on deko3d during the geometry hunt
  ("leave it off while comparing pictures"). `HANDOFF_2026-10-29.md` already listed "enable PVS
  culling for dkgl" as a next step.
- **Uploads:** `dk upload shapes` shows about one 64x64 RGBA8 upload per frame (never busy) and
  about one joint-palette sub-rectangle (`1024x15` RGBA32F, rows 2–7) per frame.
- **Late ticks:** every late tick kept is kept because its 2D list carries an image transfer
  (`late ticks still drawn: … 2D list uploads an image 90–120 of 120`).

## 2. Changes (files and reasons)

### 2.1 `OPENRAC_TERRAIN_BATCH=1`: experiment, default off

Files: `port/scene/level_renderer.{cpp,h}`, new `port/scene/terrain_batch.h`.

**What it does**
- Only the terrain's solid pass, colour only, on the CPU-culling path (not the depth prepass, the
  Vulkan GPU-driven path, colour batching, or with camera culling off).
- Terrain groups are batched when they have:
  - exactly one instance, kept by the camera;
  - blend 0;
  - the same shadow role;
  - instance data **byte-identical** to a bucket's first member (at most 16 buckets).
- Each bucket of two or more groups gets:
  - an index list of all members' solid primitives (the material-ordered model the pass already
    draws), minus metal, cutout and PVS-hidden primitives;
  - indices grouped by material, so the pass draws **one draw per material**;
  - drawing at the first member's place, with its vertex array and instance.
- The index buffer is rebuilt only when the bucket's members, models or the PVS mask change
  (FNV signature).
- The level keeps its CPU index copy when this is on, even with PVS off.

**Why it's equivalent**
- The same vertex buffer (absolute indices, no base vertex), the same triangles, material state,
  shader and instance record.
- `tests/terrain_batch_test.cpp` checks, on the host, that the batch carries every member
  triangle per material.

**Safety**
- If any piece reaches past the CPU index copy, that bucket is not batched: its members draw as
  before, so no geometry is ever dropped.
- With no CPU index copy, nothing is batched.

**Why it is opt-in**
- Fragments now draw in material order. Faces at *exactly* equal depth from different fragments
  may resolve differently (`GEQUAL`: the later draw wins).
- It is new code on the hot path, untested on hardware.

**Expected:** terrain solid draws fall from about 1637 toward the number of materials. Session 34's
cutout and translucent terrain passes already draw 73 each. The snapshot reports
`terrain batch: B batches of M groups, R rebuilt this frame`.

### 2.2 `OPENRAC_DK_PVS=1`: an existing switch, recommended as the first experiment

- No code change. A ready config is in `build/deko3d-release/pvs-test/env.txt`, and it works with
  the current NRO.
- This is the game's own conservative visibility: unknown cells or malformed words show
  everything (`scene/pvs.h`).
- **Expected:** fewer tie instances and terrain fragments, especially behind walls and terrain.
- **Check:** pop-in when crossing cell boundaries.

### 2.3 `OPENRAC_DK_PASS_STATS=1`: diagnostic, default off; needs `OPENRAC_DK_PASS_TIMING`

Files: `port/renderer/dkgl/dkgl_{internal.h,core.cpp}`.

- Each pass mark also writes five deko3d 0.5 counters: vertex-shader invocations, clipper input
  primitives, clipper output primitives, fragment-shader invocations and samples passed.
- **Storage:** 1536 reports per slot (24 KiB). Marks are reserved so every begin keeps its end.
- **Reported per pass and frame**, beside the ms.
- **What it answers:** "how much geometry is processed only to discard fragments". Fragment
  invocations above samples passed means shaded and then discarded or depth-failed. Clipper in
  above out means geometry outside the view.
- **Caveats:**
  - deko3d enables `SampleCounterEnable` at queue start, so samples passed counts;
  - the other statistics may read 0 if this GPU does not count them unenabled;
  - Claude did **not** write any enable method, because 0x509 is `InvalidateSamplerCacheNoWfi` in
    deko3d's class definition;
  - extra reports per mark: do not compare its timings with runs without it.

### 2.4 What the late ticks' image sends are

Files: `frontend/{frame.cpp,state.h,interpolation.cpp}`.

- With `OPENRAC_PACING_COUNTS`, each late tick kept because of `sends_image` has its host→GS
  transfers recorded: `BITBLTBUF` DBP/DPSM and `TRXREG` width/height.
- **New line:** `pacing: image sends in late ticks kept for them (N destinations): GS block 0x… format 0x… WxH: count; …`.
- This identifies what the uploads are before their handling changes. Behaviour is unchanged.

### 2.5 Exit path: Codex's fix reviewed, and one more path fixed

File: `port/game/common/program/vsync.cpp`.

**Codex's L+R+Minus change is right as far as host review can tell:**
- `vsync()` runs on the window/game thread;
- `leave()` is already called from four other places in `vsync()`;
- `leave()` closes sound first (documented heap corruption otherwise), then `frontend::close()`.
  That joins the render worker and resets the frontend State, whose Window destructor calls
  `dkgl::close()`: queue idle, memory blocks, swapchain (`nwindowReleaseBuffers`), device.
  Then it stops the sound readers and calls `std::exit` → `atexit`: `finish()` and
  `test_logs::archive()`.

**Claude's addition:** the HOME-menu close, `if (!appletMainLoop()) std::exit(0);`, had the same
bypass. It now takes the same route: `leave()` with a window, else stop the readers and exit.

**Why the old route is a plausible cause:**
- With a bare `std::exit`, dkgl, SDL (audio renderer) and the swapchain were torn down only by
  static destruction, in whatever order, or not at all.
- The homebrew loader (sphaira) starts in the same process after the game.
- The crash is a null data abort deep in sphaira's startup (`atmosphere-exit-crash.log`), which
  fits services or window buffers left in a bad state.

**This is not proof**, and the crash is not declared fixed.

**`test_logs` interaction:** archiving still runs from `atexit` after the orderly teardown. Codex's
revision only replaces known files in `latest/`, never follows symlinks, and recovers a crashed
run at the next start.

**If the crash persists after this build:**
1. Test with `OPENRAC_TEST_LOGS=0` to rule out archiving.
2. Test with another loader.
3. Get sphaira's matching ELF to decode `+0x2f3c58`.

### 2.6 Docs and env files

- `docs/port/DEBUGGING.md`: rows for `OPENRAC_TERRAIN_BATCH`, `OPENRAC_DK_PVS`,
  `OPENRAC_DK_PASS_STATS`, `OPENRAC_DK_PASS_TIMING`, `OPENRAC_DK_LATE_ACQUIRE`,
  `OPENRAC_STATIC_FRONT_TO_BACK`, `OPENRAC_BETWEEN_PRESENT_COST`,
  `OPENRAC_NO_BETWEEN_WHEN_BEHIND`, `OPENRAC_PACING_COUNTS` and `OPENRAC_TEST_LOGS`.
- `build/deko3d-release/<name>-test/env.txt`: each is one line different from
  `build/deko3d-release/env.txt`, which is unchanged.

| Folder | Line | Needs the new build? |
|---|---|---|
| `pvs-test` | `OPENRAC_DK_PVS=1` | No |
| `between-present-cost-test` | `OPENRAC_BETWEEN_PRESENT_COST=1` | No |
| `front-to-back-test` | `OPENRAC_STATIC_FRONT_TO_BACK=1` | No |
| `late-acquire-current-test` | `OPENRAC_DK_LATE_ACQUIRE=1` | No |
| `terrain-batch-test` | `OPENRAC_TERRAIN_BATCH=1` | Yes |
| `pass-stats-test` | `OPENRAC_DK_PASS_STATS=1` | Yes |

## 3. Not changed, and why

- **Shaders and packs:** they cannot be regenerated. The tie cost (vertex and primitive bound by
  the numbers above) can't be cut in the shader here.
- **Front-to-back:** stays opt-in (`front-to-back-test`). Whether it helps depends on the fragment
  share of the tie solid pass, which pass statistics will show.
- **Draw distances:** session 34 already used the original distances. No further reductions.
- **Upload path and Full barriers:** unchanged. The uploads are now named; the 64x64 one and the
  2D image sends need identifying first (section 2.4).

## 4. Tests (host; exact)

- **Mock dkgl** (real `dkgl_gl.cpp` + `dkgl_core.cpp` against the mocked deko3d), one clean
  process per mode. 0 failures in each:

| Mode | PASS |
|---|---|
| default | 29 |
| `PASS_TIMING=30` | 33 |
| **`PASS_TIMING=30` + `PASS_STATS=1`** | **34** (new statistics check) |
| `PASS_TIMING=30` + `PASS_STATS=1` + `LATE_ACQUIRE=1` + `DK_TIMING=30` | 36 |
| `PASS_STATS=1` (no pass timing: inert) | 29 |
| `DK_COUNTERS=1` | 30 |
| `DK_COUNTERS=30` | 30 |
| `LATE_ACQUIRE=1` | 29 |
| `DK_TIMING=30` + `LATE_ACQUIRE=1` | 31 |
| `DK_TIMING=120` | 31 |
| `TEXTURE_BIND_RUNS=0` | 29 |
| `SAMPLER_CACHE=0` | 24 |
| `LAZY_TARGET_BARRIERS=1` | 34 |
| `VERTEX_CACHE=0` | 22 |
| `PANEL_TRACE=1` | 30 |
| `SAMPLER_REBIND=first` | 29 |
| `SAMPLER_REBIND=new` | 29 |

  Pre-existing expected failures: `SAMPLER_REBIND=0` → 1; `TRANSFER_SYNC=legacy` → 3.
- **`terrain_batch_test` (new): PASS.** It checks:
  - every member triangle is kept per material (multiset);
  - one run per material, in material order;
  - order within a material kept;
  - empty pieces skipped;
  - an out-of-range piece refuses the whole batch;
  - an empty input works.
- **`-Wall -Wextra -fsyntax-only` clean (host):** `level_renderer.cpp`, `interpolation.cpp`,
  `frame.cpp`, `vsync.cpp` (without `__SWITCH__`).
- **Not compiled here:**
  - `vsync.cpp` with `__SWITCH__` (needs devkitA64 SDL2 headers): Codex should repeat its
    AArch64 `-fsyntax-only` check;
  - the Switch link;
  - `graphics.cpp`.
- **Hardware-only:**
  - batch and PVS pixel equivalence and pop-in;
  - whether the statistics counters read non-zero;
  - FPS;
  - the exit crash.

## 5. Risks and open questions

- **Terrain batch:**
  - equal-depth resolution between fragments;
  - extra CPU index copy memory (the level index list) when on;
  - rebuild cost when the camera-kept set changes (a copy of the visible terrain indices, about
    40k triangles).
- **PVS on deko3d:** its first use on this backend in a performance build.
- **Pass statistics** add reports to every mark; diagnostic runs only.
- **Exit:** sphaira's crash may have causes outside the game.
- **Carried over:** landing audio/animation timing, Pyrocitor preview, canopy across all flights.

## 6. Stock-clock test plan (one variable at a time)

Conditions for every run: 640x480, the same cap and route, original draw distances, env from
`build/deko3d-release/` (baseline `env.txt`). Logs land in
`sdmc:/switch/openrac/test-logs/latest/`.

1. **New build, baseline env.** Confirm the startup lines and the same timings as session 34.
   Exit with L+R+Minus **and**, in a second run, via the HOME menu → close. Report any crash
   report.
2. **`pvs-test`:** ties/terrain instances and ms; pop-in when moving between areas.
3. **`terrain-batch-test`:** terrain solid draws (snapshot), terrain ms, the image at the same
   spot (seams, z-fighting).
4. **`pass-stats-test`:** a diagnostic run. Report the pass lines (vertex shader, clipper,
   fragment shader, samples passed) for ties solid/cutout/translucent and terrain.
5. Then, separately: `late-acquire-current-test`, `between-present-cost-test`,
   `front-to-back-test`.
