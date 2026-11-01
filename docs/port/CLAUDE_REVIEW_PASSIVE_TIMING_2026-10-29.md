# Claude review of the passive timing / trace candidate (2026-10-29)

## Codex follow-up: reviewed and built after user authorization

The historical "not built" statements below describe the Claude session. The
reviewed revisions have now been compiled with the installed Switch toolchain.

- Reviewed frame-slot lifetime, three16-byte timestamp records, frame-triggered
  reporting, audio trace draining outside the lock, and expanded panel metadata.
- Added one guard: accept the eop->end sample only when top<=eop<=end, so an
  invalid second start cannot produce a span larger than the original.
- Updated the local mock panel assertion to accept a configurable budget rather
  than hard-coding1/32. This affects the local test, not production behavior.
- Native renderer mock passes default, DK_TIMING30 and PANEL_TRACE64 modes.
- Native sound-engine regression passes with AUDIO_TRACE off and on. This run
  did not independently repeat Claude's sanitizer suite.
- Actual NRO target succeeds with inner exit0; NRO/ELF copies hash-match outputs.
  Patched and stock shader packs match staged RomFS files. No shader changes.
- NRO SHA2568e18629ebb8d5fddfd810a5aa341d1e9b5f6d1c53f81d97dbff35362c6f12b2f.
- BuildID cf41f3c867bc832549f2314300e73d048f143732.
- Matching ELF SHA256ef2b90c1ef1a527f9aae307c86668dad6f3f030382aa35033d41e10ea7c9b7be.
- Candidate build/deko3d-test/openrac-rac1-pal-deko3d.nro; manifest updated and
  hardware_tested=false. Previous a130e669 candidate preserved under
  diagnostics/2026-10-29-before-claude-review-build/. Earlier working-graphics
  reference remains untouched under before-passive-timing/.

Assessment: appropriate diagnostic revisions. The new EOP-based span remains a
pipeline interval, not GPU busy time, and adds report commands when enabled.
No CPU wait/Full barrier was added for the extra start. Actual timing validity,
audio-event capture and panel trace relevance still require hardware results.
No claim that landing audio or Pyrocitor appearance is corrected.

---

Reviewed `HANDOFF_PASSIVE_TIMING_2026-10-29.md` and the code it describes. Four small revisions were
made, all inside the optional diagnostics. No rendering, shader, barrier, audio-playback or vendor
behaviour changed.

**Not built.** The Claude session has no devkitA64 toolchain, so these revisions are unbuilt and
untested on Switch.
- The current NRO (`a130e669…`, build ID `4348d681…`) is Codex's candidate **without** these revisions.
- The hardware-tested reference stays in `build/deko3d-test/diagnostics/2026-10-29-before-passive-timing/`.
- The pre-revision sources are in `build/claude-scratch/backup-pre-claude-timing-review-2026-10-29/`.

## Review findings

### 1. GPU reports (dkgl_core.cpp)

**Verified against the deko3d 0.5 source:**
- **Record layout.** `dkCmdBufReportCounter` writes a FourWords report for every counter except
  ZcullStats: a 64-bit counter, then the 64-bit timestamp at +8. Offsets 8/24 (and now 40) are right.
- **Units.** `dkTimestampToNs` is `ts * 625 / 384` in the SDK header. No local constant is needed.
  yuzu uses the same ratio, reverse-engineered from NVN.
- **Alignment.** 16-byte alignment is met (the memblock is `DK_MEMBLOCK_ALIGNMENT`).
- **No added waits.** Reads happen only after the slot's existing `dkFenceWait`. `submit_list`
  signals that fence with `flush=true`, so the GPU's L2 is flushed before the CPU reads uncached
  memory.
- **Slot reuse.** The memset that resets a slot's records happens after its fence, so the GPU is
  done with that block.
- **List boundaries.** A frame split into several lists keeps one start (`timing_started`).
  `flush_and_wait` neither resets nor reads the records.

**Limits (documented in the log line and here):**
- **Engine coverage.** The end report is a 3D-engine report at the end of the 3D pipe. It does not
  wait for the 2D engine's present blit or for copy-engine work, so their tails may fall outside the
  span.
  - Copy-engine transfers are already fenced by Full barriers in the 3D stream, so they are inside.
  - The present blit follows a wait-for-idle (the texture-cache workaround), so only the blit itself
    is uncertain.
- **Start report.** The original start report (`TimestampPipelineTop`, as in night100) is written
  when the command processor reaches the list. With two frames in flight it can precede the end of
  the previous frame's 3D work, so consecutive spans can overlap.

**Revision 1: two start timestamps, one report trigger.**
- **Records.** A second start report, `DkCounter_Timestamp` (end of the 3D pipe), goes at +32. The
  log gives both spans:
  - `top->end`: the original measurement, kept.
  - `eop->end`: starts after the previous frame's 3D work has drained.

  The difference between them estimates the overlap and queueing.
- **Report trigger.** The report now fires every N presented frames instead of every N valid GPU
  samples. Fence wait, acquire and present interval are therefore still logged if every timestamp
  is rejected.
- **Wording.** The log line now says these are not GPU busy time.
- **Mock check.** The command-stream mock gives 2.000 ms (`top->end`) and 1.000 ms (`eop->end`) from
  its 1 ms-per-report fake clock.

### 2. CPU timing (interpolation.cpp)

**Correct.**
- On dkgl, `render_scene` runs on the window thread.
- `scene` = draw_kept start → poses uploaded.
- `record` = all GL recording for the world, HUD and panels.
- `render CPU span` = scene + record (+ the forced wait if FRAME_COST is on, which the line reports).

Things to keep in mind:
- `record` can occasionally include dkgl's own GPU waits (`flush_and_wait`: mip audits, draw
  traces, selftests).
- Presentation is outside it. The dkgl line covers presentation: the CPU fence wait in `begin_slot`
  is time blocked on the GPU, acquire is time blocked on the swapchain, and the CPU frame interval is
  taken at acquire.

### 3. Audio trace (sound_engine.cpp)

**Locking is correct.**
- The mixer stores one atomic timestamp per traced track and does no I/O.
- All logging happens outside `g_lock`.
- The reader logs after releasing the lock.

**Gap.** The first-consumption event was only logged when the game called
`IsVAGStreamBuffered`/`GetVAGStreamTimeRemaining` afterwards. RAC1 queries music for crossfades, but
a cutscene stream may never be queried, and that is exactly the landing case.

**Revision 2.**
- Traced tracks go into a short list (at most 32, under `g_lock`).
- Game-thread calls drain it outside the lock and log consumption: `openrac_stream_play`, `_live`,
  `_stop` and every `openrac_sound_command`.
- Entries that failed, or that no stream or reader holds any more, are dropped, so decoded samples
  are not kept alive.
- Nothing runs on the mixer thread, and with `OPENRAC_AUDIO_TRACE` off the cost is one static bool
  test.
- **Tests:**
  - The existing `sound_engine_test.cpp` passes with the trace off and on (ASan/UBSan, plus TSan with
    the trace on).
  - A new local check plays a track, renders 10 ms and stops it with no query. The consumption line
    is logged now and was missing before.

### 4. Panel trace (dkgl_gl.cpp, scene_hud.cpp)

**Gap.** The trace showed `metal_pass` and the textures, but not what separates the moby passes or
whether a shine pass covers the model.

**Revision 3, for panel draws only:**
- **Per draw:** blend enable, equation, separate RGB/alpha factors, depth test/func/write and colour
  mask.
- **Pass uniforms:** `glow_pass`, `translucent_pass`, `effect_pass`, `glow_split`, `cutout`,
  `material_colour` and `fog_colour`.
- **Per-instance inputs by name.** The first instance's values, including `lights` (= -1 - shine
  distance) and `tint`, so a draw can be matched with the `panel actor` line.
- **Budget.** `OPENRAC_DK_PANEL_TRACE=N` (N > 1) raises the draw budget to N and the request budget
  to N/4. With `=1` it stays 32 draws / 8 requests.
- **Mock check.** The mock gained a check that a 1024x1024 draw logs the state, `metal_pass` and the
  instance inputs.

**Still not logged:** texture contents or hashes, and per-primitive material identity.

The shine hypothesis stays a hypothesis. A shine pass (`metal_pass 1`) drawn with SRC_ALPHA blending
over the whole model, with an instance `lights` value giving reach greater than distance, would
support it. Compare with the OpenGL build at the same vendor before changing shine semantics.

### 5. Other checks
- The geometry audit is opt-in, and the index and attribute range guards stay active: confirmed.
- The `use_level` load-span trace uses the same steady clock as the audio events: confirmed.
- Line endings were kept (`scene_hud.cpp` is CRLF).

## Local verification
- **Compile:** `dkgl_core.cpp` and `dkgl_gl.cpp` have no warnings under g++ 13 and clang++ 18
  (`-std=gnu++20 -Wall -Wextra`). `scene_hud.cpp` passes a syntax check against the PC's current
  frontend headers.
- **Mock (`transfer-sync-mocktest.cpp`, ASan/UBSan):** 0 failures by default, with
  `OPENRAC_DK_TIMING=30`, and with `OPENRAC_DK_PANEL_TRACE=1`.
- **Sound engine:** test passes with the trace off and on, under ASan/UBSan and TSan; the
  drain-on-stop check passes.

## Needs hardware evidence
1. **Timestamps and pacing:**
   - valid samples, `rejected 0`;
   - plausible spans (a few to ~20 ms);
   - `eop->end` ≤ `top->end`;
   - the CPU present-to-present average ≈ max(render CPU span + present work, GPU span) when
     GPU-bound.
2. **Report units.** Confirm on the console that the report really is FourWords with the timestamp
   at +8. If every sample is rejected, the raw values in the first four warnings show the layout.
3. **Audio trace on the landing.** Is the stream requested during the level 98 load, how long do
   read and decode take, and when does the first consumption happen relative to the level-load end?
   Compare with the audio offset measured in the clip.
4. **Panel trace at the vendor.** Is the pale look a shine pass over the model, or a missing or
   wrong `tex_T0`? Same check in the OpenGL build.

## Before the Switch test
- Build with the existing script: `build\deko3d-test\build-deko3d.bat` or `claude-build-deko3d.sh`.
- Check `=== exit code 0`, the `openrac-rac1-pal_nro` target, and that the NRO and ELF match the
  build outputs.
- Save the ELF next to the NRO (the script copies it to `last-build.elf`). Record its hash in
  `last-build.json` with `hardware_tested=false`.
- Codex's candidate `a130e669…` is replaced by that build; keep a copy if you want to compare.
