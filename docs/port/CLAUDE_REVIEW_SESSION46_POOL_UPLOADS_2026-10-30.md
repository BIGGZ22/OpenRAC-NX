# Claude review — session 46 results: pose-helper slowdown, upload differences (2026-10-30)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

Not touched:
- the tested NROs: release `7cee5db6…` / build ID `f7343d68…`, diagnostic `5c528c3a…` / `4c0ec229…`;
- their `last-build.elf/json`;
- the session 44/45 configs;
- the hardware logs.

Backups of edited files: `build/claude-scratch/backup-pre-claude-session46-2026-10-30/`. The
md5s of every original matched the session-45 outputs before editing.

**No behaviour change in this pass.** Everything new is a diagnostic behind an existing or new
opt-in switch. Defaults, the repeat-skip rule, the pool's scheduling and the helper default (off)
are all unchanged. These were left exactly as they were:
- the vendor-lettering fix;
- the frame-cap anchor;
- copy-engine sync;
- the UAM pack;
- audio.

## 1. Why one pose helper made `prepare` slower

### Established from source (not hardware)
- **Shape of the work.** The pool has one call site: `frame.cpp` `parallel::for_each(pose_jobs.size(), …)`.
  - Each piece is one moby's `scene::moby_palette` → `evaluate_moby` → `parse_frame` ×1–2 and
    `rac1::evaluate_keys`.
  - The heavy-window `prepare` is 4–8 ms serial. The scene pass counts suggest roughly 150–200
    mobys, so each piece costs tens of µs.
  - Every piece is *allocation-heavy*:
    - `evaluate_moby` builds a `MobyAnimClass` per call (`skeleton`/`rest`/`parent_word`
      `resize`);
    - it fills the `mods`, `layer_keys` and `layers` vectors and the frames' vectors;
    - `evaluate_keys` allocates `std::vector<std::array<V4,4>> rec(kRecords = 0x70)`, about 7 KiB
      zeroed, plus `post_list`, and fills 2 × 4 KiB `post_a`/`post_b` on the stack;
    - the result is a returned `std::vector<JointMatrix>`.
- **Scheduling overhead is small by construction.** Per piece there are two atomic read-modify-writes
  (`next`, `done`). Per call there is one mutex, one `notify_all` and one completion wait. At about
  200 pieces of tens of µs each, that is around 1% of `prepare` even if every RMW misses cache.
- **False sharing.** `next` and `done` share a line, so the caller and helper bounce it. That costs
  on the order of 100 ns × 2 × 200 ≈ 0.04 ms a tick: it cannot explain +5 ms.

### Hypothesis, not proven: contention in the allocator
On the Switch, newlib's `malloc`/`free` take one process-wide lock (`__malloc_lock`, a libnx
mutex). An uncontended take is a user-space CAS. A contended one goes to the kernel
(`svcArbitrateLock`/`Unlock`), costing µs, and wakes the waiter on another core.

With the helper there are three threads allocating at once:
- the game thread running pieces;
- the helper running pieces;
- the render worker recording.

The pieces allocate dozens of times each, so the two pose threads would hand the lock back and
forth continuously. That would make each piece slower on *both* threads, and could make two
threads slower than one. That matches `prepare` going from 4–8 to 10–13 ms with no affinity failure
reported.

Host tests cannot confirm it: glibc uses per-thread arenas and doesn't serialise like newlib.

Other candidates are not excluded:
- core 2's L2 is shared by all four cores, so the pieces gain no cache;
- the helper may wake late;
- the runs differed in route and camera, and the scenes were not matched.

### What this pass adds to settle it: `OPENRAC_POOL_STATS=N`
`port/common/parallel.cpp`, a diagnostic, off by default. It logs one line every N calls that had
helpers (or every N calls when there are none):

```
parallel stats over N calls (K helper(s)): P pieces a call, caller ran C, helpers H (none in X calls);
a call W ms, of which the caller's own pieces A ms (a µs a piece) and waiting for the helpers' last B ms;
helpers busy Y ms a call (y µs a piece), first piece S ms after the call began
```

How to read it, comparing run 01 (serial) with run 02 (one helper):

| Observation in run 02 vs run 01 | Means |
|---|---|
| µs a piece (caller and helper) clearly **above** run 01's caller µs a piece | Pieces slow each other: shared-resource contention (allocator lock most likely). Fix = fewer allocations per piece, not scheduling. |
| µs a piece about equal, but `first piece` late (ms) or helpers ran few pieces | Wake-up latency / helper not getting the core. Fix = keep the helper spinning briefly, or fall back to serial under a minimum count. |
| µs a piece about equal, `waiting for the helpers' last` large | One long piece at the end. Fix = smaller tail / caller steals. |
| Sum of busy time ≈ run 01 but wall time not halved | Overhead outside pieces (publish/notify/wait). |

Cost: two clock reads per piece on helpers only, and four per call. Nothing when unset.

Helpers record their stats per piece, before `done.fetch_add`, so the caller reads complete values
once it has seen `done == count`. TSan finds no race.

**Not done, deliberately:** chunking, a minimum-work threshold or a serial fallback. The evidence
so far points at the work inside a piece, not at the scheduling. Chunking would not reduce the
allocations, and changing the scheduler without the numbers would be guessing.

**Optimisation candidate once run 02 confirms contention.** It reduces CPU even serially.
- Allocation-free pose evaluation:
  - reuse per-thread scratch for `rec` and `post_list`;
  - pass the output palette in instead of returning a new vector;
  - keep a `MobyAnimClass` per moby class instead of rebuilding it per moby per tick.
- It touches `assets/geometry/moby_animation.cpp` and `scene/moby_pose.cpp`, which are shared with
  the desktop renderer. It needs a pose-equality test over captured RAM, so it is left for a
  separate reviewed pass.

## 2. Which uploads keep a frame from being skipped

Session 46 results: decoder unread 0; heavy verdicts "different" 104–109 per window; repeat 0. The
whole-list equality refuses the skip, but it doesn't say *why*.

**Added (diagnostic only, under `OPENRAC_PACING_COUNTS` + `OPENRAC_LATE_IMAGE_REPEAT_SKIP`):**

- **`image_sends::first_difference(now, was)`** (in `image_sends.h`, unit-tested) classifies where
  a late tick's transfer list first differs from the last drawn tick's:
  - **number of transfers**: one list ends early, or the counts differ;
  - **a transfer's place**: block, width, format, corner or rectangle. A sub-flag records whether
    the two lists hold *the same transfers in another order*;
  - **only a transfer's bytes**: same place and size, different content hash or received length.
- **Per destination, for each decoded late image tick.** The key is
  `{block, format, w, h, occurrence in tick}`. It counts:
  - how often it was seen;
  - how often its bytes changed since the last decoded tick;
  - how many distinct contents it had this run.

  Bounds: 48 destinations and 8 contents each, for the whole run. The map is static, so nothing
  grows.
- **New pacing lines** each window:
  ```
  pacing: different sends, first difference from the last drawn tick's: number of transfers X, a transfer's place Y (of them the same transfers in another order Z), only a transfer's bytes W
  pacing: transfer destinations by how often their bytes change (N destinations; contents counted to 8 per run): block 0x… format 0x… WxH #k: changed c of s (d contents); …
  ```

**How to read them:**
- **Mostly "only bytes", with few destinations of few contents**, e.g. a palette cycling through 4
  contents: real animated textures or palettes. The frame genuinely needs those uploads.
  - The safe follow-up would be to process transfers while skipping the draw. The renderer's
    `TexturePool` state after the tick would then match. It needs the interpreter's upload path
    without the GS draw state, so it is a separate design.
- **Mostly "place, same transfers in another order"**: an ordering artifact. Even then, order
  matters wherever destinations overlap, so it is not to be ignored without an overlap proof.
- **"Number of transfers"**: uploads that come and go (streamed HUD pieces).
- **A destination with `contents` hitting 8 every window**: likely a streamed or video-like
  surface.

**What the skip itself compares is unchanged.** No difference is ignored, and nothing about order,
overlap or GS state was relaxed.

## 3. CPU critical path

Measured shares in session 45/46, per heavy tick:
- game logic: 12–14 ms;
- prepare: 4–8 ms serial;
- capture: 2.4–2.9 ms.

`prepare` is the one stage in this port's own code that is clearly allocation-bound (section 1).
That is the concrete next optimisation once the stats line confirms it. Game logic is the
recompiled game itself, and capture was not profiled finer this pass. Worker early submission is
untouched and stays a separate experiment.

## 4. Audio caution (no change)

The session 46 audio trace shows about 1000 s of wall time against 35–54 s of mixer time and
21–31 s of game time in late windows. That is a pause, suspension or trace-lifetime discontinuity,
not drift. No audio code changed. Queue semantics are preserved, and the 7 s first sound is still
explained by queued playback.

## Files

| File | Change |
|---|---|
| `port/common/parallel.cpp` | `OPENRAC_POOL_STATS`: per-call/per-piece caller vs helper time, helper start latency, final wait; `drain()` returns pieces run |
| `port/tests/parallel_test.cpp` | nested call, two concurrent callers × 20 rounds × 5000 items, 400 × 180-piece calls (all exactly-once) |
| `port/game/common/frontend/image_sends.h` | `Difference`, `FirstDifference`, `first_difference()` |
| `port/game/common/frontend/frame.cpp` (CRLF) | `note_difference()` (first difference + bounded per-destination change counts) |
| `port/game/common/frontend/state.h` (CRLF) | `diff_*` counts, `destination_changes` |
| `port/game/common/frontend/interpolation.cpp` | the two pacing lines |
| `port/tests/image_sends_test.cpp` | `first_difference` cases (same, shorter, bytes, swapped = order-only, moved, inserted, partial) |
| `port/tests/env_test.cpp`, `docs/port/DEBUGGING.md` | `OPENRAC_POOL_STATS` |

## Tests (Linux container, g++ 13 / clang 18)

**`parallel_test`**
- TSan with `OPENRAC_POOL_THREADS` unset/0/1/2/3 × `OPENRAC_POOL_STATS` unset/30: all exit 0,
  0 TSan warnings.
- With stats on there are 20–21 stats lines.
- ASan/UBSan with 0/1/2/3 helpers and stats 30: all exit 0.
- What it covers:
  - exactly-once;
  - exception rethrow;
  - nested calls (serial inside a body);
  - concurrent callers (one gets the helpers, the other runs alone);
  - many short calls;
  - pool teardown at exit (join).

**Other tests and checks**
- **`image_sends_test`**: g++ `-O1` and `-O2` with `-Wall -Wextra -Wconversion` and ASan/UBSan;
  clang `-O2`. All exit 0, 0 warnings.
- **`env_test`**: exit 0. The new row and the direct-reader list agree.
- **`tick_phases_test`** (unchanged code, rerun): off, and on ×3 under TSan. All exit 0.
- **`moby_passes_test`** (unchanged, rerun): ASan/UBSan, exit 0.
- **Syntax and warnings:**
  - `frame.cpp` and `interpolation.cpp`: `-fsyntax-only -Wall -Wextra -DOPENRAC_FRONTEND`, gcc and
    clang, clean;
  - `parallel.cpp` with `-D__SWITCH__` against libnx headers, gcc and clang, clean.
- **Sound tests:** not rerun, since no sound code changed.

**Not done here:**
- the AArch64 build;
- the Windows runs;
- the full host suite. Its `common/system_info.cpp` problem is unrelated and unresolved, so there
  is no full-suite claim.

## Hardware plan (two runs, `build/deko3d-release/session46-tests/`, NEW build only)

| Run | Config | Read |
|---|---|---|
| 1 | `01-serial-stats`: session-45 `01-repeat-skip-live` + `OPENRAC_POOL_STATS=120` | Serial µs a piece and pieces a call. The new "different sends" and "destinations" lines: kind split, and top destinations with their change rate and number of contents. |
| 2 | `02-one-helper-stats`: 01 + `OPENRAC_POOL_CORES=1` | The same lines. Caller and helper µs a piece vs run 1, helper first-piece latency, final wait, pieces split. Same route and camera as run 1 where possible. |

Both runs:
- note the clocks the user set (the logs don't verify them);
- note any pause or suspend, which spoils audio wall time.

Advice for play until then: helper off (session-45 `01-repeat-skip-live`).
