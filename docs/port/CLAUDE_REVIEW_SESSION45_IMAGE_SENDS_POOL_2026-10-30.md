# Claude review — session 44 results: image-send cuts proven dropped, pose pool (2026-10-30)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

What was not touched:
- the tested NROs: release `30fa1eda…`/`2527e0c0…`, diagnostic `89081c36…`/`cf57cd4a…`;
- the configs.

Backups of edited files: `build/claude-scratch/backup-pre-claude-session45-2026-10-30/`.

What stays as it was:
- the vendor-lettering fix and the worker frame-cap fix are unchanged;
- the user confirmed the GADGETRON ring is fixed, which validates that ring only, not every cutout
  texture.

## A. Why every gameplay image path was "not read cleanly" — established

**Capture.** `build/deko3d-test/worker-fixes-session44/image-sends-fixture.bin`: tick 1455, 47 184
bytes, SHA256 `e0f0d527…a1fc6`. It is game-derived and stays local: not copied into the repository
and not published.

**Method.** I built a local host harness (`build/claude-scratch/session45-harness/`, see below):
- the real `GifInterpreter::vif` from `port/renderer/direct.cpp`, unmodified;
- `TexturePool::upload` replaced by a recorder at link time;
- the GL calls left inert.

It runs the captured path through the interpreter and through `image_sends::scan`, and compares
every upload: place, format, size and bytes.

**Result on the capture.**
- The interpreter and the scan delivered **the same 13 uploads, byte for byte**:
  - 16x16 CT32 palettes;
  - 32x32/64x64 PSMT8 textures;
  - one 64x64 CT32 at 0x3317.
- No read-backs, local copies or offscreen FRAME writes.
- All 12 problems are `CutImage`: IMAGE data past its DIRECT with no transfer open.

**The bad boundary.** DIRECT 8 is 48 bytes (3 qwords), right after a FOGCOL (0x3D) A+D DIRECT. Its
bytes are texel-like grey RGBA: `94 94 94 14 | 89 89 89 0a | 8e 8e 8e 0e | 8d 8d 8d 0d …`. Read as a
GIF tag, the first qword is IMAGE, NLOOP 5268, NREG 16, far longer than the DIRECT. The same
48-byte blocks recur at DIRECTs 12, 14–16, 18, 43, 44, 52, 54, 55 and 58: two distinct contents,
repeated.

**What the runtime does with them.** `GifInterpreter::gif` in a VIF DIRECT: "IMAGE data with no
transfer open → `cut()`": the rest of the DIRECT is dropped, nothing is written, and no state
changes. The interpreter's own comment names the cause: "a reference to memory the port has put
other bytes in".

**Root cause (behaviour).** The decoder was right to flag these, but they are bytes the renderer
drops too. The renderer's state cannot depend on them. The repeat-skip's question is "does skipping
this tick lose anything the renderer would have received?" These bytes don't affect the answer.

Not proven: why those DIRECTs point at texels. Probably packet buffers the port's native HUD
routines leave as they were; it is not needed for the rule.

**Is the decoder faithful in general? A differential fuzz.**
- Random paths shaped like `direct_packets` output, run through both:
  - transfers with IMAGE tags split or short;
  - raw data, random A+D writes, long PACKED and IMAGE tags, REGLIST;
  - TRXDIR 0/1/2/3.
- **It found one real divergence:** the decoder closed an open transfer on TRXDIR 1 (read back).
  The interpreter acts only on directions 0 and 2, so the open transfer stays open. Fixed, with a
  regression test. That case was always rejected as `kReadBack`, so it never made an unsafe skip.
- **Comparison detail:** the interpreter records a CT32 upload into the draw buffer twice (the
  second keyed at 0x10000+, `draw_frame_upload`), so the harness ignores those copies.
- **After the fix: 0 mismatches** in 520 000 paths, 6 seeds, about 450 000 uploads and 2.3 million
  cut tags, under ASan/UBSan.

**Rule (behaviour change, only under the existing opt-in `OPENRAC_LATE_IMAGE_REPEAT_SKIP=1`)**
- `image_sends::read_as_renderer(scan, strict)` accepts paths with tags cut off by their DIRECT
  (PACKED, REGLIST, IMAGE-without-transfer).
- It still refuses:
  - a misread path (header not DIRECT, data past the end, trailing bytes); `direct_packets` never
    makes these;
  - image data still open at the end, which the renderer would carry into the next frame.
- All the other vouchable conditions are unchanged: complete transfers, no read-back, no local
  copy, no offscreen FRAME, drawn reference, same sends byte for byte.
- Transition invalidation is unchanged.
- **Reference `OPENRAC_LATE_IMAGE_REPEAT_STRICT=1`** restores the old refusal.
- **New counter:** repeats whose paths had dropped tags.

Expected hardware effect:
- in heavy windows, late ticks whose HUD uploads repeat the last drawn tick's are now left out
  (≤3 in a row);
- game ticks per second should rise; displayed frames per tick will fall below 1 by design;
- watch the HUD, pause, map and vendor screens for stale or missing pictures.

**Also fixed:** a GCC 13 `-O2` `-Wstringop-overflow` false positive in `Reader::direct`. It now
copies into a buffer sized once.

## B. Pose pool — measured 0 helpers; placement made bounded and testable

Session 44 logged "parallel: 0 pool thread(s) … hardware threads reported: 0". So
`hardware_concurrency()` is 0 on the Switch, and the 4.9–6.6 ms of pose preparation ran on the game
thread alone. The core layout changes between runs (game on core 0 with the worker on 1 in session
44; reversed before).

**Change (`common/parallel.cpp`)**
- **`OPENRAC_POOL_CORES=N`** (Switch, opt-in, N 1..3) now means N helpers, on the allowed cores
  other than the creator's, highest first. The render worker takes the lowest such core, so
  `OPENRAC_POOL_CORES=1` puts its one helper on the core both leave free:
  - mask 0x7, game 0 → worker 1, helper 2;
  - game 1 → worker 0, helper 2;
  - nothing hardcoded.
- **Before**, `=1` meant every allowed core except the creator's, so 2 helpers with one sharing the
  worker's core. The existing `session44-tests/02-pool-cores` config on the *current* NRO still
  runs the old meaning.
- **`OPENRAC_POOL_THREADS=N`** (0..3, any platform) sets the helper count without moving them. It
  exists for host tests and comparisons.
- **Tested:** `parallel_test` with 0/1/2/3 helpers under TSan (6 stress runs with 3, 0 warnings)
  and ASan. These are the first runs that exercise the pool's threads (this host reports 2 hardware
  threads).

Defaults: unchanged (no helpers on the Switch). No new default without hardware evidence.

## C. Not done

**Worker early scene submit.** It touches dkgl slot and fence ownership and UI ordering. With the
GPU at 10–14 ms against a 21–24 ms tick, the tick is bound by game + prepare + capture, so it would
mainly cut latency. Left for a separate opt-in.

Kept as they are:
- copy-engine `Full` barriers;
- the patched UAM pack;
- queue/loop audio semantics. The first-sound delay is confirmed queued playback ("queued behind 1
  part of 1555874 with about 7070 ms left").

## Files

| File | Change |
|---|---|
| `port/game/common/frontend/image_sends.h` | `read_as_renderer()`; TRXDIR 1 keeps an open transfer open; join without vector insert |
| `port/game/common/frontend/frame.cpp` (CRLF) | `repeat_strict()` reference; `read_as_renderer` in `vouchable`/`repeat_verdict`; `repeat_with_dropped` count |
| `port/game/common/frontend/state.h` (CRLF) | `repeat_with_dropped` |
| `port/game/common/frontend/interpolation.cpp` | verdict line shows it |
| `port/common/parallel.cpp` | `PoolPlan`; `POOL_CORES=N`; `POOL_THREADS=N` |
| `port/tests/image_sends_test.cpp` | synthetic fixture-shaped path; `read_as_renderer` cases; read-back keeps transfer |
| `port/tests/env_test.cpp`, `docs/port/DEBUGGING.md` | `POOL_THREADS`, `LATE_IMAGE_REPEAT_STRICT`; updated `POOL_CORES` and `REPEAT_SKIP` rows |

Local harness (not built, not for commit), in `build/claude-scratch/session45-harness/`:
- `harness.cpp`: fixture vs interpreter;
- `fuzz.cpp`: differential fuzz;
- `seams.cpp`: TexturePool recorder and inert GL pieces;
- `README.txt`.

## Tests (Linux container, g++ 13 / clang 18)

All of these pass:
- `image_sends_test`: `-O1` and `-O2`, `-Wconversion`, ASan/UBSan, 0 warnings.
- `parallel_test`: TSan with 0/1/2/3 helpers; ASan.
- `env_test`.
- Fixture harness: 13/13 uploads identical.
- Fuzz: 0 mismatches in 520 000 paths.

Syntax and warnings:
- `-fsyntax-only -Wall -Wextra`: `frame.cpp` and `interpolation.cpp` (gcc and clang), clean.
- `parallel.cpp` with `-D__SWITCH__` against libnx `svc.h`: clean.

Not checked here:
- a full AArch64 build;
- the Windows runs.

## Hardware runs (new build; `build/deko3d-release/session45-tests/`)

| Run | Config | Check |
|---|---|---|
| 1 | `01-repeat-skip-live`: session-44 run 01, fixture off | Verdict line (repeat count, "repeats whose paths had tags the renderer drops too"), left-out counts, ticks a second, HUD/pause/map/vendor. If the HUD looks wrong, repeat with `OPENRAC_LATE_IMAGE_REPEAT_STRICT=1`. |
| 2 | `02-pool-one-helper`: 01 + `OPENRAC_POOL_CORES=1` | The pool line names one helper on the free core; `prepare` ms and ticks a second against run 1. |

Open, not claimed:
- the Pyrocitor preview;
- the canopy across flights;
- broad PVS visuals;
- the exit crash.
