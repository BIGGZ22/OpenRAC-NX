# Claude review — session 49: the changing 64x64 texture, next measurements (2026-11-01)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

What stays as it was:
- the tested NROs (release `ee758930…` / ELF `55bf2134…`, diagnostic `899d3037…` / ELF `30ac5129…`);
- the configs;
- the session-48 pose work;
- the upstream ports.

Backups: `build/claude-scratch/backup-pre-claude-session49-2026-11-01/`.

## 1. Session 49 in brief

- **Pose verify:** 7116 poses checked, 0 differences. That supports the reused scratch and RAM
  reading on the route tested. The arithmetic is shared with the reference, so it is not
  independent proof of `evaluate_keys`.
- **User-confirmed on hardware:**
  - the Pyrocitor preview is textured;
  - fades no longer leave an edge line;
  - animation looks right.
- **Still untested:** the Quartu flight and the Suck Cannon gauge.
- **Serial pose cost:** 23.5–32.4 µs a piece, against 40–43 µs in session 47.
  - This is not a controlled comparison: the route, class mix and camera differ, and diagnostics
    were on.
- **Load:** mixed CPU and GPU.
  - game CPU 15–19 ms, prepare 4–7 ms, capture about 3 ms;
  - heavy GPU command spans 18.7–23.5 ms (p95 up to 27.8 ms), with occasional slot waits;
  - CPU and GPU spans overlap, so they are not summed.
- **Diagnostic hitches** came from the owner trace's 32 MiB RAM search and the reference
  re-evaluation. A clean measurement run is needed.

## 2. The changing 64x64 texture

### Established

**A. Where the 0x3317 match came from.** The earlier owner line that put "3 effect strips" on
block 0x3317 was a CLUT match, not the same texture.
- The first strip's TEX0 `0x800662e59930731b` decodes as:
  - TBP0 **0x331b**, PSMT8, 64x64, TBW 1;
  - **CBP 0x3317**, CPSM CT32, CLD 4.
- That is exactly what `GetEffectTex` (`func_001F4868`, `games/rac1/src/game/draw.c`) builds: the
  CLUT at the GS texture cursor, then texels 0x400 bytes (4 blocks) later.
- So an effect texture's palette sat at the first block of the region the 64x64 CT32 transfer
  covers (0x3317..0x3356), and its texels started 4 blocks in.
- The old trace counted a strip when TBP0 *or* CBP equalled the transfer's block. **Two different
  textures sharing GS memory** therefore looked like one.
- The new trace (below) counts base matches, CLUT matches and overlaps separately.

**B. The consumer of 0x333f.** The strips whose TEX0 base *is* the transfer's block
(`TEX0 0x59810333f`, TEX1 `0xFF9000000260`, ALPHA `0x2000000064`, CLAMP 0) use the same GS setup as
the game's environment-mapped meshes.
- On Novalis, `draw_level01_environment_mapped_meshes` (`overlays/l01_novalis/vendor_002FABE8.c`)
  does the following:
  - `VU1_addGSregister(6, GetEffectTex(0x28))`;
  - ALPHA `0x2000000064`;
  - TEX1 `0xFF9000000260`;
  - one `func_L00_001FDE48` call per mesh. That function is `DrawEnvOverlayMesh` in `names.tsv`,
    "draws the 1848 meshes", and it emits the strip packets `direct_packets` reads as effect
    strips.
- ALPHA `0x2000000064` is (Cs − Cd)·0x20/128 + Cd, a fixed 25% blend over what is already drawn.

**C. TEX0 `0x59810333f`** decodes as base 0x333f, PSM field 1 (CT24, so alpha comes from TEXA, not
the texels), TW = TH = 6, TCC 1. The TBW field is 0. That is unusual, but it is what the game
wrote; nothing here interprets it.

### Hypothesis, not proven

On Kerwan the per-tick 64x64 grey texture is the **environment or reflection map of an
environment-overlay mesh** (class 1848 or similar): sampled by `DrawEnvOverlayMesh` strips and
blended 25% over the scene.

What is not established:
- that it is drawn on every tick it is uploaded;
- which function writes it.

Kerwan's own draw callback is not in the decompilation's C (only 7 Kerwan files are), so the call
site was not read.

### Producer: not found

The bytes match RAM 0x1dc4d0. That is inside the region where level overlays load (level data such
as `D_L11_001DAB40` sits nearby), and the port relocates level data addresses. Neither the C nor the
generated host C has 0x1dc4d0 as a literal, and a RAM match alone proves nothing.

The new trace instead logs **the game address the DMA chain actually read the transfer's first
image byte from, and the DMA tag that sent it**. That separates a buffer the game points the DMA at
(REF/REFE) from bytes copied into the packet (CNT/NEXT), and gives the packet's location for a
follow-up.

## 3. Change: owner trace corrected and made cheap (`frame.cpp`, diagnostic only)

- **Strip matching** is now split by how a strip involves the transfer's blocks
  `[dbp, dbp + bytes/256)`:
  - TEX0 base at the block, in the transfer's format;
  - TEX0 base at the block, in another format;
  - indexed texture whose CLUT lies inside the blocks;
  - texels only overlapping.

  The registers printed are those of the first base match only.
- **Source:** while the trace is on, `direct_packets` keeps the chain's pieces in
  `ImageSourceTrace` (game-thread static, cleared per path, one entry per tag that sent data):
  VIF offset, game address, bytes, tag address and tag id. Per destination block it also keeps the
  VIF offset of a transfer's first image byte and the TEX0 and ALPHA set before it.
  - This covers a first byte in the IMAGE tag's DIRECT and one running on into the next DIRECT.
  - The owner line names the game address, the piece, and the tag kind and address.
- **The 32 MiB RAM search is removed.** That was the hitch.
- **Unchanged:** the 64-transfer bound, level clearing, the texel figures from strip uploads,
  read-only behaviour, and off unless `OPENRAC_IMAGE_OWNER_TRACE=N`.

**Checks:**
- `frame.cpp`: `-fsyntax-only -Wall -Wextra`, gcc and clang, clean.
- Not executed here: there is no host harness for `direct_packets`. Codex should check the log
  line on hardware.
- **Cost when on:** the per-tag vector, plus one `image_sends::scan` per tick. Nothing when off.

## 4. A transfer-only skipped tick: design, not implemented

**Why not now.** It changes what the render worker receives for a skipped tick, so it needs:
- render-worker API and slot review;
- a build;
- a hardware A/B.

That is too much to do blind, and it should not be mixed with diagnostics.

**Design (for a later, separate, opt-in switch such as `OPENRAC_LATE_IMAGE_TRANSFERS_ONLY=1`):**

1. **Eligibility** (frontend, at `leave_late_frame`), checked in this order:
   - Keep every existing refusal: unread, incomplete, read-back, local copy, offscreen FRAME, no
     reference, level change, at most 3 in a row.
   - The tick's verdict is `kDifferent`.
   - The transfer list differs from the last drawn tick's **only in bytes**: same count, same
     places in the same order (`first_difference` gives `Bytes`).
   - Session 47's logs show this is the common case: 105–113 a window were bytes-only.
2. **Worker:**
   - Send the tick as a *transfer-only frame*: the same packet buffers, flagged.
   - Run the real interpreter: `DirectRenderer::submit(packets)`, the code path that calls
     `TexturePool::upload` and tracks pending images across DIRECTs.
   - Then drop the draw lists with `m_interpreter->clear()`, with no `flush()`, no target binds and
     no present.
   - Effect strips' uploads (`effect_uploads`) must go to the pool the same way.
   - **Reuse the interpreter. Do not model the GS separately.**
3. **State to preserve:**
   - upload order and destination rectangles: the interpreter does this;
   - the pending image carried into the next DIRECT: the interpreter;
   - the TexturePool generation and age, so a later sample sees the newest bytes;
   - the reference state (`g_drawn_sends`), updated to the transfer-only tick's list, since the
     renderer now holds it.

   GS register context is not carried between frames by the frontend; check it in
   `GifInterpreter::clear`.
4. **Fall back to a full draw when:**
   - the worker cannot take the flagged frame;
   - the slot is busy past its normal wait;
   - any `kReadBack`, `kLocalCopy` or `kOffscreen`;
   - the path has a world marker split that the interpreter needs for the native world;
   - the effect quads' notes (#95) are involved: clear them as a skip does now.
5. **Differential test** (host, on the session-45 harness, using the real `GifInterpreter`):
   - For random and captured paths A, B, C:
     - (draw A, transfer-only B, draw C) must leave the pool's uploads identical to (draw A, draw B,
       draw C);
     - C's draws must be identical either way.
   - Plus the refusal cases.
6. **Expected effect:** it saves B's draw passes and presentation GPU work. It does not save the
   game CPU or the capture. It helps only when the tick is late.
7. Hardware A/B with the switch off and on; HUD, pause, map and vendor checks.

## 5. Pose helper and clean measurement: two runs on the CURRENT NRO (no build needed)

`build/deko3d-release/session49-tests/`:

| Run | Config | Purpose |
|---|---|---|
| 1 | `01-serial-clean` (session-48 `01` without `POSE_VERIFY` and `IMAGE_OWNER_TRACE`) | Clean passive figures: tick phases, pose µs a piece, GPU spans, presents a tick. No periodic hitches. |
| 2 | `02-helper-verify` (01 + `OPENRAC_POOL_CORES=1` + `OPENRAC_POSE_VERIFY=60`) | Pose helper after the allocation reuse. Caller and helper µs a piece against run 1. 0 differences. The verify work runs outside the pool call, so µs a piece is unaffected. |

Run 1 also answers priority 4's first question, the size of the real costs without diagnostic
overhead.

The corrected owner trace needs the new build: `03-owner-trace-new-build` = 01 +
`OPENRAC_IMAGE_OWNER_TRACE=600`, on Kerwan and wherever 0x333f shows.

## 6. CPU and GPU profiling (priority 4): plan

- **Game CPU 15–19 ms.** This is the recompiled game itself. Finer splitting needs per-function
  sampling, and `OPENRAC_GAME_TIMING`'s existing categories are the starting point. Run 1's clean
  numbers come first.
- **GPU 18–23 ms in heavy windows.** `OPENRAC_DK_PASS_TIMING` already splits passes. Read run 1's
  per-pass lines before choosing a GPU change. Candidates from the other ports, each behind a
  switch:
  - render-target compression;
  - drawing straight into the swapchain;
  - lean barriers.

  None is started here.
- **Early worker submit** remains a separate opt-in, with slot, fence and UI ownership review. It
  cuts latency, not game CPU.

## 7. Upstream audit correction (from Codex)

`_cast_exe_call` changed **14 call sites in 4 generated files**, not only `scale_game_frames`:
- the files: `core/00119D88.c`, `game/pause.c`, `game/space.c`, `game/vendor.c`;
- the targets: `func_00118E70` (1), `001153FC` (4), `001F98C0` (1), `00120F30` (1), `00118D80` (3),
  `001160D8` (1), `00122598` (1), `0012DDC0` (1), `0012E2E8` (1).

All are casted `OPENRAC_CODE` calls to executable-only functions that became direct calls, under
the guard (exe-only places, prototyped, not variadic, no split, no fold, no level hook). For an
exe-only function, the direct call is what the cast meant. The Quartu bug was a level function
answering at the exe address. Full diff: `build/deko3d-test/upstream-generated-diff.txt`.

Regression checks:
- pause menu;
- vendor buy and sell;
- the core paths those functions belong to;
- the flight.

## Files

| File | Change |
|---|---|
| `port/game/common/frontend/frame.cpp` (CRLF) | `ImageSourceTrace` pieces and sources in `direct_packets`; `note_image_owners` rewritten (match kinds, source, no RAM search); `texture_bytes()` |
| `docs/port/DEBUGGING.md` | `OPENRAC_IMAGE_OWNER_TRACE` row |
| `build/deko3d-release/session49-tests/*` | configs |
