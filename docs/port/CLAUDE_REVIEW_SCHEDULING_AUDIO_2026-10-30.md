# Claude review — tick scheduling, late image sends, landing audio (2026-10-30)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push. Nothing
here is hardware-verified. The tested NROs/ELFs/manifests (performance `3e357444…`/`93fd2a7c…`,
diagnostic `fe1e7379…`/`e9d50c19…`) and `build/deko3d-release/terrain-pacing-pvs-test/env.txt`
(session 39 reference) are untouched. Backups of every edited file:
`build/claude-scratch/backup-pre-claude-scheduling-2026-10-30/`.

Evidence read: HARDWARE_SESSION36/38/39/40 notes, BUILD_TERRAIN_EXIT, and the archived game logs
in `build/deko3d-test/pvs-session39/` and `late-acquire-session40/`. Routes differ between
sessions: these are workload samples, not identical-camera A/B measurements.

## A. Why the GPU gets a frame only every ~29–33 ms

**deko3d draws every frame on the game thread.** `draw_kept` (interpolation.cpp) uses the render
worker only when `gl::is_vulkan()`; dkgl's GL_VERSION is `"4.1 OpenRAC on deko3d"`, so on the
Switch build the whole frame runs on the game thread, one stage after another:

1. the game's own code for the tick: 11.5–12.7 ms ("game wall interval outside host vsync");
2. vsync: reading the tick (`frontend::frame`), then `render_scene` (scene + record: 5.0–5.5 ms,
   `OPENRAC_CPU_TIMING`), then `present_frame` (dkgl submits the scene only here, then acquires,
   blits and presents);
3. back to the game's code.

The GPU receives one frame per tick and is idle until the next present. In session 39 at frame
1500:
- present to present: 29.3 ms;
- GPU span: 11.5 ms;
- gap: 17.9 ms;
- game code: 11.9 ms.

So vsync takes about 17.4 ms, of which `render_scene` is 5.5 ms. The **other ~12 ms (capture,
present, the rest of vsync) were never measured.** The new `OPENRAC_TICK_PHASES` measures them.

What it is not:
- **The GPU.** The span is about 12 ms, against a 16.7 ms tick.
- **The screen-image wait.** Acquire takes 0.2 ms, the slot-fence wait is about 0, and session 40's
  late acquire left the gap where it was.

**Threads on the Switch stay on their creator's core.** libnx starts a `std::thread` on the
default core (its creator's), and nothing in `port/` calls `svcSetThreadCoreMask`. These all share
the game thread's core with the game code:
- the Vulkan render worker;
- the stream readers;
- probably SDL's audio thread (not verified).

**Interpolation eligibility.** `between()` predicts a frame's cost from `pacing.cost`, the CPU time
before the swap, so it never sees the GPU or the present. This is why session 36 drew 60 extra
frames per 120 ticks while every tick was late.

`OPENRAC_NO_BETWEEN_WHEN_BEHIND` is principled rather than blunt:
- a frame between ticks exists to fill refreshes while the next tick is not yet due;
- when the tick is already half a tick late, there is nothing to wait for;
- it never delays a tick.

I recommend keeping it in the reference, and making it the default once menus, map and pause are
confirmed unchanged at 60. No default changed here.

## B. Late ticks kept because they "upload an image"

**The diagnostic was wrong.** `note_image_sends` (and the behavioural `sends_image`) read every
16-byte step of the 2D path as an A+D register write. The path is `NOP NOP NOP DIRECT` headers
followed by GIF data, and the GIF data holds:
- PACKED RGBAQ/XYZ2 qwords, whose ninth byte is a colour's blue or a depth's low byte;
- IMAGE payload.

Any of these can read as TRXDIR (0x53), BITBLTBUF (0x50) or TRXREG (0x52). That produced the logged
"block 0x2406 format 0x31 578x578", 2901x3363 and "0x0 0x0" entries.

**What the old reading misses and adds:**
- It never misses a real A+D TRXDIR: DIRECT data is qword-aligned, and REGLIST's 4-bit descriptors
  cannot name the transfer registers.
- It can add false ones. So its decision (keep the frame) is conservative.
- The behavioural default is unchanged.

**New decoder** (`frontend/image_sends.h`): reads the path tag by tag as
`GifInterpreter::vif`/`gif` do.
- A+D writes are taken only from PACKED tags.
- REGLIST and IMAGE are stepped over by size.
- Image data runs on into the next DIRECT only while a transfer is open.
- A register tag cut off at the end of its DIRECT drops the rest of that DIRECT.
- It records destination, rectangle, expected and received bytes, and a content hash.
- It counts read-backs, local copies, offscreen FRAME writes and cut or malformed data.

`note_image_sends` now uses it, and a new pacing line reports how many kept ticks the decoder finds
sending nothing (the old reading's false match).

**What is probably real.** The entries that look sane recur every late tick:
- two 64x64 CT32 images to block 0x3317;
- a 16x16 CT32 palette and a 64x64 PSMT8 texture pair (0x3357/0x335b, 0x336b/…).

This looks like the HUD's textures being sent again every tick, and the fixed decoder will
confirm it. If so, `sends_image` keeps almost every late tick, `leave_late_frame` never leaves one
out, and the game's time runs at the display's rate: 29–30 ticks a second of 60 in heavy gameplay.

**Experiment `OPENRAC_LATE_IMAGE_REPEAT_SKIP=1`.** A late tick is no longer kept for an image when
its decoded path is vouchable and either:
- sends nothing; or
- re-sends exactly what the last drawn tick sent: same places, sizes, bytes and order.

Vouchable means:
- no cut or malformed tag;
- no incomplete transfer;
- no read-back;
- no local copy;
- no render to texture.

Why repeats are safe:
- re-uploading the same bytes to the same places is idempotent;
- the reference is taken only when the tick's own frame is actually drawn (`image_sends_drawn` from
  `draw_kept(first)`, matched by `Kept::frame`);
- a frame drawn without a decode, or a level change, invalidates the reference.

The usual limits still apply: at most 3 frames left out in a row, `has_camera`, `index > 1`.
Expected effect: in heavy scenes, game ticks rise toward 60 a second while fewer frames are shown.

## C. Landing animation/audio delay

There are two separate mechanisms, and they pull in opposite directions:

1. **Slow ticks against real-time audio.** The port ticks at 60 Hz ("PAL 50 Hz: the port keeps
   60"). Heavy gameplay ran at 29–30 ticks a second, so game time ran at about half real time.
   Streams play in real time, because the audio device pulls the mixer. Tick-driven animation
   therefore falls behind its audio by about 0.5 s per second of scene. This matches "animation
   delayed", and it shrank as tick rate rose (13 → 20 → 30).
2. **A stream starts late.** `read_track` reads and decodes the whole file before its first sample
   can sound. A speech track is about a megabyte from SD, and the reader shares the busy game
   thread's core. This is a fixed start delay that grows with load, and matches the earlier
   "audio delayed".

Neither has been measured on the landing. Changes:
- **`OPENRAC_AUDIO_TRACE` additions:**
  - the request tick;
  - the first-sound tick (in ticks and game seconds after the request);
  - about once a second while a traced stream plays, "audio played X s, the game advanced N ticks
    = Y s; audio ahead of the game by D s" (`openrac_sound_trace_tick`, called once a tick from
    vsync).

  A constant D means a start offset; a growing D means slow ticks. The position is the mixer's,
  which runs ahead of the speaker by the output buffer.
- **Experiment `OPENRAC_STREAM_EARLY_START=1`:** `read_track_early` reads and decodes in parts
  (first 160 frames, then 4096), publishing `Track::available` under `g_lock`. The vector is sized
  once before publication. The mixer plays only `available` samples and holds silent if it catches
  the reader. The samples are identical: same decoder, rounding and history.
- **Default `read_track`** sets `available = size, complete = true`, so the default mix is
  unchanged.
- **No global audio delay** was added.

## D. env-applied.log caveat

`env::load_switch_file` writes `env-applied.log` when it reads env.txt at the start of `main`.
`test_logs` copies that file into run folders at exit, and copies an unclean previous run's files
when the next run starts, after the new run has already rewritten it.

I did not prove why the attached copies were older than the active settings. Instead, the game log
now records the effective environment itself: `log::info("environment in effect: OPENRAC_…")` at
session open (main.cpp, Switch only). Use that line rather than the attachment.

## Changes (all opt-in or diagnostic; defaults unchanged)

| Switch | Default | What | Files |
|---|---|---|---|
| `OPENRAC_TICK_PHASES=N` | off | Game-thread tick split into game, prepare, capture, render, worker wait, present, between, sleep, other (mean/p95; sums to the whole tick) | NEW `frontend/tick_phases.h`; vsync.cpp, interpolation.cpp |
| `OPENRAC_DK_RENDER_THREAD=1` | off | deko3d uses the existing render worker (as Vulkan does): world recorded while the game runs its next tick; present and UI stay on the game thread a tick later. `OPENRAC_RENDER_THREAD=0` still forces synchronous | interpolation.cpp, `gl::is_deko3d()` (gl.h/.cpp) |
| `OPENRAC_RENDER_CORE=N/-1` | auto with DK_RENDER_THREAD | Switch: worker moves to core N, or the first allowed core other than the game thread's (`svcGetInfo(CoreMask)`, `svcSetThreadCoreMask`); logs the result; failure leaves it in place. Vulkan worker unchanged unless set | render_worker.h/.cpp |
| `OPENRAC_LATE_IMAGE_REPEAT_SKIP=1` | off | Section B | frame.cpp, state.h, interpolation.cpp, NEW `frontend/image_sends.h` |
| `OPENRAC_STREAM_EARLY_START=1` | off | Section C | lib/sound_engine.cpp |
| `OPENRAC_AUDIO_TRACE=1` | off | Tick view added | lib/sound_engine.cpp, openrac/sound_engine.h, vsync.cpp |
| (diagnostic) | on with PACING_COUNTS | Decoded image-send list; false-match and undecodable counts | frame.cpp, interpolation.cpp |
| (diagnostic) | Switch | "environment in effect" line in the game log | main.cpp |

Also changed:
- the late-frame counters and the decode are now computed only when `OPENRAC_PACING_COUNTS` is set
  (before, the counters ran always and were only reported with it);
- DEBUGGING.md has rows for the six new variables, plus a note on the `OPENRAC_RENDER_THREAD` row;
- `env_test` lists the six as direct-getenv readers;
- new tests are registered in CMakeLists.txt and `cmake/Games.cmake`.

## Host tests (container, g++ 13 / clang 18; ASan+UBSan unless noted)

- **`image_sends_test`** (new): PASS. It covers:
  - real uploads with hashes;
  - HUD palette+texture pairs;
  - colour/depth false matches that the old qword reading reports and the decoder rejects;
  - texels containing 0x53;
  - IMAGE split across DIRECTs;
  - REGLIST stepping;
  - cut tags;
  - copies, reads and offscreen FRAME writes;
  - malformed paths;
  - unsized formats.
- **`tick_phases_test`** (new): PASS off and with `OPENRAC_TICK_PHASES=30` (3 runs). Phases sum to
  the whole tick within 0.01 ms, and spans exclude their nested phases.
- **`sound_stream_early_test`** (new): PASS with early start off and on, also with
  `OPENRAC_AUDIO_TRACE=1`:
  - same pinned hash `2bd2a9fa9628097b` in both modes;
  - threaded read equals read-at-once;
  - mixing while the reader runs plays every sample and ends;
  - a truncated file sounds what exists with early start, and nothing without (as before).

  TSan: 0 warnings, 3 runs per mode.
- **`sound_engine_test`** (existing): PASS with early start off/on, trace off/on; TSan clean.
- **`render_worker_test`** (existing + placement case):
  - PASS under TSan and UBSan;
  - under ASan both the original and the new file abort in libasan's sigaltstack teardown
    (`crash::prepare_thread`), which is pre-existing and unrelated.
- **`env_test`** (updated): PASS against the updated DEBUGGING.md and sources.
- **`-fsyntax-only -Wall -Wextra`, 0 warnings:**
  - interpolation.cpp, frame.cpp, vsync.cpp, render_worker.cpp and gl.cpp, with and without
    `OPENRAC_FRONTEND` (SDL 3.2 headers);
  - the same files under clang;
  - render_worker.cpp with `-D__SWITCH__` against libnx's real `svc.h`/`result.h`/`types.h`
    (gcc+clang, `-Wconversion`).
- **Not compiled here:**
  - main.cpp's `__SWITCH__` block (`environ` from newlib `<unistd.h>`; the snippet itself compiled
    on Linux);
  - any full AArch64 translation unit.

  Codex: AArch64 syntax-check main.cpp, render_worker.cpp, interpolation.cpp, frame.cpp, vsync.cpp
  and sound_engine.cpp.

## Risks and hardware-only questions

- **DK_RENDER_THREAD:** dkgl has no thread-local state (checked), and ownership is handed over
  under the worker's mutex. Still unproven on hardware:
  - no `stats::kGpu` scope or dkgl call runs on the game thread while a job is out (the Vulkan
    design already assumes this);
  - the present happens one tick later, adding one tick of latency;
  - in-flight snapshots and dkgl's 2 slots with the scene submitted at present.

  Watch for: HUD/menu/vendor-screen errors, crashes on level load and transitions, the exit path.
- **RENDER_CORE:**
  - applet mode may allow fewer cores, so the log line says what was allowed;
  - SDL's audio thread and the stream readers are not moved.
- **LATE_IMAGE_REPEAT_SKIP:**
  - fewer frames are shown by design;
  - a scene that uploads once and draws from it later is protected by the drawn-frame reference,
    but not proven on every screen. Check the HUD, pause, map, vendor and ship flights.
- **STREAM_EARLY_START:**
  - a starved reader means silent holds (gaps) rather than a late start;
  - a truncated file plays its readable part instead of nothing.
- **Tick phases:**
  - "present" includes dkgl's present work and swap;
  - "between" includes pacing's own waits;
  - GPU time is not CPU time.

## Ready env files (`build/deko3d-release/`)

Each is the session 39 reference plus the listed lines. The reference file is unchanged, and all
of these need the new build.

| Folder | Adds |
|---|---|
| `tick-phases-test` | `OPENRAC_TICK_PHASES=120`, `OPENRAC_AUDIO_TRACE=1` (diagnostic baseline) |
| `dk-render-thread-test` | the two above + `OPENRAC_DK_RENDER_THREAD=1` |
| `late-image-repeat-test` | the two above + `OPENRAC_LATE_IMAGE_REPEAT_SKIP=1` |
| `stream-early-start-test` | the two above + `OPENRAC_STREAM_EARLY_START=1` |

Order:
1. `tick-phases-test`: gameplay, then the ship landing. It gives the phase split, decoded image
   sends and the audio-vs-tick drift.
2. `dk-render-thread-test`.
3. `late-image-repeat-test`.
4. `stream-early-start-test`: the landing again.

Compare these in the logs:
- "ticks a second";
- `game`/`capture`/`present`;
- the GPU gap;
- the pacing lines;
- the audio trace's "ahead of the game" values.

Open issues carried over:
- exit crash not confirmed fixed;
- PVS visual validation across levels/cells;
- Pyrocitor preview material;
- canopy on all flights.
