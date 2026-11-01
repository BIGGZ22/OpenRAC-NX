# Claude review — vendor lettering, worker pacing, image-send reasons, audio queue (2026-10-30)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

What was not touched:
- the tested scheduling NROs (release `a23e6d18…`/`66cc4f1b…`, diagnostic `f35610a6…`/`a08333cf…`);
- the reference and three-run configs.

Backups of every edited file: `build/claude-scratch/backup-pre-claude-session44-2026-10-30/`.

Inputs:
- `CLAUDE_NEXT_SESSION43_2026-10-30.txt`;
- `VENDOR_HOLOGRAM_LETTERING_2026-10-30.md`;
- the archived logs of sessions 41–43 (baseline, rendering, audio);
- the user's request to finish the scheduling work, look into the Gadgetron lettering, and improve
  what needs it.

## 1. Gadgetron vendor lettering (class 1143) — root cause found, fix by default

**Evidence**
- **Upstream fixed this twice on Oct 16:**
  - `e549d6f04`: the glow pass blends a glow packet's part-transparent texels;
  - `758a3bedb`: cutout moby textures get one mip level and bilinear filtering, because "a mipmap
    chain averages a cutout's alpha away at a distance (thin, broken letters)".

  Its commit message reports the band "reads whole" on Novalis with both in place.
- **The second fix recognises moby textures by name.** `level_renderer.cpp` tests
  `image_paths[i].find("/textures/moby_")`, which is the GLB export's file name.
- **The Switch never uses that name.** It builds its levels straight from the disc
  (`scene/disc_import.cpp`, added Oct 27), and names every image `native/<table>/<index>`. Moby
  textures are `native/1/N`: table 1 of `decode_level_textures`, and a moby class's materials take
  group 1. The test never matches, so on the Switch every cutout moby texture kept its mipmap
  chain. The hologram's lettering is mostly alpha 0x22..0x5E around thin opaque strokes, so it
  thins to lines.
- **This is why every Switch build shows it** (OpenGL, Vulkan, deko3d alike), regardless of the
  shader pack.
- **The shader side matches.** The pack is looked up by a hash of the exact runtime sources, and
  `missing programs 0` in every session means the deko3d program is the current `mesh.frag`,
  glow-pass fix included. OpenGL shows the same symptom, so the pack is not the cause.
- **Other parity checks:**
  - moby clamping: both importers repeat for mobys; the editor clamps only terrain, ties, shrubs
    and sky;
  - cutout classification: both use "any texel below 250";
  - alpha doubling: `assets/image.h`, 0x80 → 255;
  - glow vertices: packets from `glow_high` on.

**Fix (default on)**
- `scene/moby_passes.h`: `moby_texture_path()` accepts either name.
- `level_renderer.cpp` uses it.
- It logs how many disc-imported cutout moby textures now get one level.
- Reference `OPENRAC_NATIVE_CUTOUT_MOBY_MIPS=1` restores the old mipmapped behaviour.

Scope:
- it affects only cutout moby textures on disc-imported levels: the hologram, fences and foliage on
  mobys;
- it is the GLB path's existing behaviour, as the GS samples moby textures (TEX1 MXL 0);
- geometry and rotation are untouched;
- no rotation-only change was made.

**Not verified on hardware.** If the ring is still wrong:
- run `OPENRAC_VENDOR_LOGO_TRACE=1` to confirm the lettering primitive's material, image and cutout
  flag, and the moby's mode bit 0x10 (glow colour);
- compare with `03-references`.

## 2. Counter disagreement (session 43, item B) — a real dropped-frame bug

**Mechanism**
- With the render worker, a frame is presented one tick after it starts.
- `present_frame` passed the frame cap a start time shifted to the present, so the next frame
  became due 16.7 ms after the present, minus the recording time.
- The next tick's frame starts a few milliseconds after that present: it is not yet due.
- `draw_tick_frame` therefore deferred it (`first_pending`), and `between()` is declined while
  behind, so the frame was **never drawn**.

Effect:
- 0.58–0.90 presents a tick in sessions 42–43 heavy windows (tick phases count real presents);
- the pacing line still said "tick frames drawn 120", because `count_tick_frame(false)` ran
  whether or not the frame was drawn.

A step-by-step simulation of ~22 ms ticks with this rule drops alternate frames, which matches the
logged 0.65–0.72.

**Fix**
- `swapped()` now takes the frame's real start (`output.started`) for the cap. On the game thread
  this is the same instant as before.
- Reference `OPENRAC_FRAME_CAP_FROM_HANDOFF=1` restores the old reckoning.
- **Counting:**
  - a tick frame counts only when drawn;
  - one the cap held back counts when `between()` draws it, and is no longer also counted as a
    between frame;
  - the pacing line adds "held back by the frame cap and drawn between ticks N; held back and never
    drawn M".

**Is the simulation faster?** Tick-phase "ticks a second" counts real vsync calls (game updates).
The faster ticks are real (the audio trace's game-time advance agrees: 14 s → 20 s over the same
26.4 s). What was missing was displayed frames, not ticks. The game's speed is unchanged.

## 3. Image sends rejected in gameplay (item D) — reasons now recorded, not relaxed

Facts:
- **`vouchable()` is not relaxed.**
- **Each unclean path now records why** (image_sends.h `Problem`, first place):
  - header not DIRECT;
  - DIRECT past the end;
  - trailing bytes;
  - PACKED cut;
  - REGLIST cut;
  - IMAGE past its DIRECT with no transfer open;
  - image data still open at the end.

  It also records the DIRECT index and size, the tag's position and both GIF-tag words.
- **Pacing lines** now show:
  - the counts by first problem;
  - under `OPENRAC_LATE_IMAGE_REPEAT_SKIP`, each late image tick's verdict (repeat, unread,
    incomplete, read-back, local copy, render to texture, no reference, different).
- **The first 6 unclean paths are logged in detail.**
- **`OPENRAC_IMAGE_SENDS_FIXTURE=1`** writes the first unclean path (≤1 MiB, once) to
  `sdmc:/switch/openrac/image-sends-fixture.bin`, as a regression fixture.

Hypothesis, not proven: the interpreter's own comment says DIRECT data can carry bytes that are not
GIF data ("a reference to memory the port has put other bytes in"), and it drops the rest of such a
DIRECT. If the reasons show that kind of cut (`CutPacked`/`CutImage` past the port's own HUD
replacement data), then:
- the decoder already mirrors the interpreter's drop exactly;
- accepting those paths would need a reviewed, opt-in rule;
- a real fixture test should come first.

## 4. Landing audio (item C)

**First sound:** sector 1555962 was requested with `queue_on 0x40000002`, i.e. queued on a live
stream. A queued part sounds only when the part before it ends. A looping part, with flags 0x24
including loop bit 4, ends at the end of its current pass. The ~7 s from request to first sound
matches queued playback, not loading. Early start made the file ready in 8.7 ms in session 43, with
no change to the first sound.

**Trace addition:** "queued behind N part(s) of sector S with about X ms left (the last loops…)"
when a traced stream is queued, so the next run states this directly.

**Drift:** the audio-vs-game drift grows because game time runs slower than real time. The render
worker roughly halved it (12.4 s → 6.4 s at 26.4 s), and that is the only real lever. No audio
offset was added.

**Tests:** new checks in `sound_stream_early_test`, in both modes:
- a part queued on a live stream starts after it;
- a looping part keeps looping until a part is queued, then ends at the end of its pass;
- all samples play once per pass.

## 5. Pose threads (new opt-in)

The tick-phase `prepare` row is the moby poses (`parallel::for_each` over `moby_palette`). In heavy
gameplay it takes 3.3–4.9 ms on the game thread. The pool's threads, if any, start on the creating
thread's core: no core placement exists in `parallel.cpp`. `hardware_concurrency()` on the Switch
is unknown.

Changes:
- The pool now logs its size once (`parallel: N pool thread(s) … hardware threads reported: H`).
- `OPENRAC_POOL_CORES=1` (Switch, opt-in) makes one pool thread per allowed core other than the
  creator's (highest first, so core 2 when the game thread is on 1 and the render worker on 0).
  Each thread moves itself there with `svcSetThreadCoreMask`.

## 6. Tick-phase test on Windows

`tick_phases_test` now spins instead of sleeping. Windows sleeps in 15.6 ms steps, which broke
6 bounds there. The accounting checks are unchanged.

## Not done

- **Early scene submission from the worker.** The GPU still starts at present, a tick later. In
  heavy windows the tick is limited by game + prepare + capture (11.6 + 3.5–4.7 + 2–4 ms), with the
  GPU at 10–13 ms. Submitting earlier would mainly cut latency; a candidate after the pool
  experiment.
- **Capture cost.** Not touched.

## Files

| File | Change |
|---|---|
| `port/scene/moby_passes.h` | `moby_texture_path()` |
| `port/scene/level_renderer.cpp` | uses it; reference switch; log line |
| `port/tests/moby_passes_test.cpp` | path cases |
| `port/game/common/frontend/interpolation.cpp` | frame-cap anchor + reference; counting in `between()`; new report lines |
| `port/game/common/frontend/frame.cpp` (CRLF) | `draw_tick_frame` returns drawn; held-back counting; `RepeatVerdict`; reason logging; fixture writer |
| `port/game/common/frontend/state.h` (CRLF) | `deferred`, `deferred_drawn`, `unread_by`, `repeat_verdicts` |
| `port/game/common/frontend/image_sends.h` | `Problem`, `ProblemAt`, per-kind cut counts, `directs`, `image_unfinished` |
| `port/game/common/lib/sound_engine.cpp` | queued-behind trace line |
| `port/common/parallel.cpp` | pool size log; `OPENRAC_POOL_CORES` |
| `port/tests/image_sends_test.cpp` | reason cases (cut kinds, unfinished image, not-DIRECT, past end, trailing) |
| `port/tests/sound_stream_early_test.cpp` | queue and loop semantics |
| `port/tests/tick_phases_test.cpp` | spin instead of sleep |
| `port/tests/env_test.cpp` | 5 more direct readers (incl. Codex's `OPENRAC_VENDOR_LOGO_TRACE`) |
| `docs/port/DEBUGGING.md` | rows for the 5 |

## Host tests (Linux container, g++ 13 / clang 18)

All of these pass:
- `moby_passes_test` (ASan/UBSan).
- `image_sends_test` (ASan/UBSan, `-Wconversion`).
- `sound_stream_early_test`:
  - early start off/on × trace off/on (ASan/UBSan), same pinned hash;
  - TSan off/on: 0 warnings.
- `sound_engine_test`: off/on.
- `tick_phases_test`: off, and on ×5.
- `parallel_test` (TSan). This host reports 2 hardware threads, so the pool has 0 threads and its
  thread path is not exercised here.
- `env_test`.

Syntax and warnings:
- `-fsyntax-only -Wall -Wextra`, 0 warnings: `frame.cpp` and `interpolation.cpp` (gcc + clang,
  `OPENRAC_FRONTEND`); `level_renderer.cpp`, with stand-ins for the two generated shader headers.
- `parallel.cpp` with `-D__SWITCH__` against libnx's `svc.h`/`result.h`/`types.h`: clean.

Not checked here:
- a full AArch64 build;
- the Windows host test runs.

## Recommended runs (`build/deko3d-release/session44-tests/`, new build, ≤3)

| Run | Config | What to check |
|---|---|---|
| 01 | `01-worker-fixes` (session-42 config + `OPENRAC_IMAGE_SENDS_FIXTURE=1`) | Heavy gameplay, a Gadgetron vendor (ring lettering), the landing. Compare with session 42: presents a tick (should be near 1.00 where ticks are drawn), "held back" counts, unread reasons, queued-behind line. Keep `image-sends-fixture.bin`. |
| 02 | `02-pool-cores` (01 + `OPENRAC_POOL_CORES=1`) | Same route. Pool log line, `prepare` ms, ticks a second. |
| 03 | `03-references` (01 + both reference switches) | Only if run 01 is unclear: the old cap and the old lettering. |
