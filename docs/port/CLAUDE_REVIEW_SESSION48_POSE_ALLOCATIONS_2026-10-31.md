# Claude review — sessions 47/48: pose temporaries reused, the changing 64x64 transfer (2026-10-31)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

Not touched:
- the tested NROs: release `1835e32e…` / ELF `214bf562…`, diagnostic `51540e46…` / ELF `c7f5e520…`;
- `last-build.elf/json`;
- the session 44/45/46 configs;
- the hardware logs.

Backups of edited files: `build/claude-scratch/backup-pre-claude-session48-2026-10-31/`.

## 1. What sessions 47/48 show (from the logs)

- **Serial (47):** 30–45 µs a pose piece, 65–166 pieces a call.
- **One helper (48):**
  - caller 156–436 µs a piece, helper 58–163 µs a piece;
  - the helper's first piece 7–11 µs after the call starts;
  - the final wait 6–44 µs.

  Wake-up and tail cost microseconds, not the milliseconds lost. Both threads' pieces got several
  times slower while running side by side.
- **Caveat on the caller's figure:** it is publish-to-drained, so it includes the notify.

**Root cause: still a hypothesis.** Pieces interfere *while working*. The pose code allocated
about 26 times per piece: 25.7 in the synthetic test; frames' four vectors per key, the class's
three vectors, the modifier and layer lists, the 7 KiB records, the post lists, the output.
newlib's `malloc` takes one process-wide lock, and a contended libnx mutex is handed over through
the kernel (`svcArbitrateLock`/`Unlock`). Two pose threads plus the allocating render worker would
convoy on it. This fits the numbers but is not proven.

Not excluded:
- per-class cost differences between the caller's and helper's pieces;
- different routes;
- instrumentation differences.

## 2. Change: pose evaluation without per-pose allocations (default on, reference switch)

### What changed

- **`assets/geometry/moby_animation.{h,cpp}`**
  - `parse_frame_into(base, offset, MobyFrame&)`: the same checks, failure messages and bytes as
    `parse_frame`, into existing vectors.
    - `read_array_into` mirrors `ByteView::read_array` exactly.
    - `parse_frame` is now this into a new frame. `parse_sequence` uses `parse_frame`.
  - `post_scale_list_into(a, b, out)`; `post_scale_list` wraps it.
  - `EvalScratch`: the 0x70 scratchpad records and the three post lists.
  - `evaluate_keys_into(…, EvalScratch&, out)`: the **same arithmetic, moved, not rewritten**.
    - Every evaluation starts by zeroing all 0x70 records. That matters: the parent chain can read
      a record past the joints, or one not yet built, exactly as the zero-initialised vector gave.
    - The lists are cleared before use.
  - `evaluate_keys` = `evaluate_keys_into` with a new scratch (heap, as the old records were) and a
    new vector.
- **`scene/moby_pose.{h,cpp}`**
  - `moby_palette_into(ram, moby, out)`: `evaluate_moby` re-expressed over a per-thread
    `PoseScratch`. It keeps the class's skeleton/rest/parent vectors, two keys, 12 layer keys,
    6 layers, the modifier list and an `EvalScratch`.
    - Same reads in the same order, same checks, same key-swap and t rules.
    - A layer is kept only when its keys are there, with its key slots taken in order, as before.
  - The old function stays as `moby_palette_reference` (unchanged code).
  - `moby_palette` and `moby_pose_matrices` are **unchanged** (reference path) for their other
    callers.
- **`game/common/frontend/frame.cpp`** (CRLF): the pool's pose loop calls `moby_palette_into` into
  a `static thread_local` vector of palettes kept from tick to tick.
  - It is resized to the tick's job count, and each entry is rewritten or emptied before it is
    read.

### Cache: none, deliberately

There is no class or pose cache. Everything is read from game memory on every call:
- a class moved or reloaded at the same address;
- a reused moby slot;
- new keys or blends;
- layers or modifiers coming or going.

All of these are seen at once, so no staleness is possible. A per-class skeleton cache would need a
byte compare as large as the read it saves, and the profile shows the reads are not the cost (see
below).

### Lifetime and ownership

- `PoseScratch` belongs to one thread: a `thread_local std::unique_ptr`, made on first use and
  freed when the thread ends. It is never shared.
- A call made while the thread's scratch is in use gets its own temporary scratch (`busy` flag,
  RAII). Nothing nests today; the busy fallback is not exercised by a test.
- **Bounded:**
  - fixed counts (2 + 12 keys, 6 layers, at most 32 modifiers, 0x70 joints);
  - after each call, any vector holding more than 16 KiB is released (`trim`);
  - the frontend's palette vector holds one entry per pose job of the last tick.

### Reference switch

`OPENRAC_POSE_REFERENCE=1` makes `moby_palette_into` the old evaluation (new allocations every
call).

### On-device check

`OPENRAC_POSE_VERIFY=N`: on every Nth tick the frontend evaluates each pose again with the
reference and compares bit for bit (`memcmp` of the matrices). It logs a `pose verify` summary
every 10 checked ticks and up to 8 detailed differences. It runs after the pool call on the game
thread, with game memory unchanged in between. That gives equivalence on real game data (every
class, layer and modifier the route meets) without RAM dumps.

### Evidence

- **Bit-for-bit against the ORIGINAL sources** (local harness, not committed). 86 400 palettes were
  printed by the original `moby_pose.cpp`/`moby_animation.cpp` and by the new ones:
  - 24 seeds;
  - palettes, pose matrices, and palettes again after a transition on the same slot;
  - 82 641 non-empty palettes, all distinct.
  - **Identical.** The same holds with the new `moby_palette_into` in place of `moby_palette`.
- **`tests/moby_pose_test.cpp`** (new, with `tests/moby_pose_ram.h`):
  - 6000 random mobys, 5772 with a palette, on one thread's reused scratch;
  - 3000 transition steps on one slot (class swapped for another joint count at the same address,
    class pointer cleared and restored, t rewritten, key bytes changed under the same pointer);
  - 4 threads × 20 rounds against precomputed answers;
  - `evaluate_keys_into` with a dirty scratch versus `evaluate_keys`;
  - `parse_frame_into` into a used frame versus a new one, with the same exception for a bad frame;
  - `post_scale_list_into` on a used list.
  - Result: **0 differences.**
- **Allocations a palette** (test-only `operator new` counter, readable keys): **reference 25.74,
  reused 0.00**.
- **Synthetic data covers:**
  - 1–0x70 joints;
  - missing, all-zero, malformed and cut-off keys;
  - parent words naming later, absent and non-record slots;
  - modifiers in mode 0 and blend modes, with invalid records and loops;
  - layers with missing keys and zero weight;
  - NaN and out-of-range t.

  **It is not every class the game has.** `OPENRAC_POSE_VERIFY` is the real-data check.
- **The existing `tests/geometry/moby_animation.cpp`** compiles and passes against both the old and
  new sources (host, ASan/UBSan).

### Expected effect: honest scope

**Host profile (x86, glibc):** about 43 µs a palette either way. 71% of the time is `mat3` and
`mat4_point` (the bit-exact PS2 multiply-adds of the parent chain and the bind combine). glibc's
allocator is cheap and uncontended here.

On the Switch:
- The serial gain is whatever ~26 newlib malloc/free pairs a piece cost: probably a small
  percentage, **unmeasured**.
- The point is the helper case. If allocator contention is the cause, run 2's per-piece µs should
  fall close to run 1's. If they stay several times higher, the hypothesis is weakened, and the
  pool should stay off for good.

The arithmetic itself is the PS2 model and was not touched.

### Pre-existing hazard (found, NOT changed)

The modifier loop in `evaluate_moby` stops at 32 *kept* modifiers or an unreadable node. A
modifier list that loops through nodes which are all skipped (no joint record) would never end.
The same goes for a layer list that loops through only rejected layers. The game links no such
lists, and the new code keeps the same loops so that results stay identical. A visit bound would
be a separate, reviewed robustness change. The test generator avoids such loops.

## 3. The changing 64x64 transfer (Priority 2): what is known, and a diagnostic

**From the session-44 fixture** (local, tick 1455, not published):
- The 64x64 CT32 upload to block 0x3317 comes in one A+D block.
- **The GS state with it:**
  - `ALPHA1 = 0x20_00000064`: (Cs − Cd)·FIX + Cd with FIX 0x20, a 25% blend;
  - `TEX0_1 = 0x598103317`: TBP0 0x3317, TW = TH = 6, TCC 1, PSM field 0x1, TBW 0, as written;
  - `FOGCOL 0x606060`;
  - BITBLTBUF/TRXPOS/TRXREG/TRXDIR (dbw 1, CT32, corner 0,0);
  - `TEST1 0x5370b`: alpha test GEQUAL 0x70, fail keeps RGB; Z test GEQUAL.
- Then 16 KiB of IMAGE data, then TEXFLUSH.
- **Its texels:** grey cloud-like noise: mean RGB 141 (0x8D) in all three channels, range 129–157, alpha 0–30 (mean 13).
- **No draw in the readable part of the path samples 0x3317.** The rest of that DIRECT range is
  bytes the renderer drops.
- **Observation, not proven:** the session-45 "cut" DIRECT bytes (`94 94 94 14 | 89 89 89 0a …`)
  look like this same grey, low-alpha noise.

Its purpose is **unestablished**. It is not called HUD here; a fog, haze or cloud overlay is one
possibility among others.

**Diagnostic added: `OPENRAC_IMAGE_OWNER_TRACE=N`** (`frame.cpp`, off by default, reads only).
- It tracks 2D-path transfers by **full identity**: block, dbw, format, corner, size, and order
  among the transfers to that block. At most 64 are tracked, cleared on a level change.
- Every N ticks it logs an `image owner` line for the transfer that changes most:
  - how many of the ticks seen it changed;
  - the effect strips of that tick whose TEX0 TBP0/CBP is its block (count, vertices, the first's
    TEX0/TEX1/CLAMP/ALPHA);
  - when a strip took the bytes: mean RGB, alpha range, and the game-memory addresses holding the
    same bytes. Mapped to symbols with the game's map, that names the buffer and so the routine
    that writes it.
- The memory search is 32 MiB on that one tick: a short hitch.

**Not done:** a transfer-only skipped tick. It would need the GS context, copy, pending-image and
ownership state preserved, and its own design and tests. Not mixed with the pose change.

## Files

| File | Change |
|---|---|
| `port/assets/geometry/moby_animation.h/.cpp` | `parse_frame_into`, `post_scale_list_into`, `EvalScratch`, `evaluate_keys_into`; the old entry points wrap them |
| `port/scene/moby_pose.h/.cpp` | `moby_palette_into` (per-thread temporaries), `moby_palette_reference`, `pose_reference()`, `pose_verify_every()` |
| `port/game/common/frontend/frame.cpp` (CRLF) | pool loop → `moby_palette_into` into kept palettes; `verify_poses`; `note_image_owners` |
| `port/tests/moby_pose_test.cpp`, `port/tests/moby_pose_ram.h` | new |
| `port/CMakeLists.txt` | `openrac_add_test(moby_pose …)` |
| `port/tests/env_test.cpp`, `docs/port/DEBUGGING.md` | `OPENRAC_POSE_REFERENCE`, `OPENRAC_POSE_VERIFY`, `OPENRAC_IMAGE_OWNER_TRACE` |

## Tests (Linux container, g++ 13 / clang 18)

**`moby_pose_test`:** exit 0 under each of:
- g++ `-O1` with ASan/UBSan;
- g++ TSan, 0 warnings;
- clang `-O2` (this container has no clang sanitizer runtime).

**Other tests:**
- Original-versus-new harness: 86 400 identical.
- `tests/geometry/moby_animation.cpp`: ASan/UBSan, exit 0 against the new and old sources.
- `env_test`: exit 0.

**Syntax and warnings:**
- `frame.cpp`: `-fsyntax-only -Wall -Wextra` (`-Wshadow` adds only the existing `level_renderer.h`
  warning), gcc and clang, clean.
- `moby_pose.cpp`: `-Wall -Wextra -Wshadow -Wconversion`, 0 warnings.
- `moby_animation.cpp`: gcc clean. clang `-Wconversion` reports only the existing sign conversions
  in `encode_snapshot`, which is untouched.

**Not run:**
- the AArch64 build;
- the Windows tests;
- the full host suite (the `system_info.cpp` problem is unrelated);
- sound tests (no sound change).

## Hardware plan (`build/deko3d-release/session48-tests/`, NEW build)

| Run | Config | Read |
|---|---|---|
| 1 | `01-pose-serial-verify`: session-46 01 + `OPENRAC_POSE_VERIFY=60` + `OPENRAC_IMAGE_OWNER_TRACE=600` | `pose verify … 0 differ` throughout; serial µs a piece versus session 47; `image owner` lines (identity, strips, memory address); visuals of animated mobys (vendor hologram, Ratchet's weapon arm, NPC heads, Gadgetron lettering) and level changes |
| 2 (only if 1 is clean) | `02-pose-one-helper-verify`: run 1 + `OPENRAC_POOL_CORES=1` | caller and helper µs a piece against run 1 and session 48; still 0 differ |
| optional | `03-pose-reference-optional` | only if a moby looks wrong in run 1 |

Defaults: the pool stays off. Verify and owner trace are diagnostics, not defaults. On the verified
ticks, tick-phase figures include the second evaluation.
