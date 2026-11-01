# Passive timing and targeted diagnostics: Claude review handoff

## Scope and protected reference

User asked to start implementing the review recommendations and leave notes and
a prompt for Claude to review/revise. Current source includes all prior work;
do not reset the working tree or rerun old integration scripts.

Known-good graphics/sampler/transition build before this pass:
NRO SHA2568595c8b64b39a7c76e9ec67c6acdc308911878d4686eb19504cb7d8e1a765f2c,
ELF buildID f1f92f0758fb299346ba8f083060731ef63064c7. It was tested in session27:
21/21 GPU tests, corrected geometry, no prior solid purple/cyan frames found,
sampler refreshes reduced about81% per draw. Profiling averages improved but
routes differ. See DKGL_FLASH_AND_SAMPLER_2026-10-29.md for exact limitations.

Preserved NRO/ELF/manifest for this pass in
build/deko3d-test/diagnostics/2026-10-29-before-passive-timing/.
The new candidate is not hardware tested. No shader, blend, culling, geometry,
transfer ordering, audio decoding/playback policy or vendor material fix is
intended in this pass. Keep the previous hardware reference for comparison.

## Changes

### 1. Verbose geometry audit is now opt-in

dkgl_gl.cpp audit_geometry checks OPENRAC_DK_GEOMETRY_AUDIT=1. Default is off.
This avoids the 12,504 geometry-audit lines observed in session27, each previously
flushed to disk. Existing indexed byte-range guards and prepare_draw attribute
offset guards remain active. Sampled max-index/fetch validation is part of the
optional audit, so don't describe it as exhaustive validation when disabled.

Mipmap and palette draw traces retain their existing switches/defaults; the new
env-passive-timing.txt explicitly sets MIP_AUDIT=0 and DRAW_TRACE=0 for measurement.
The shared logger still flushes each line. We did not globally buffer logging or
weaken driver crash diagnostics. Tiny automatic GPU selftests remain unchanged.

### 2. Optional nonblocking CPU reports

OPENRAC_CPU_TIMING=120 enables reports in interpolation.cpp report_frame_cost,
independent of FRAME_COST. Uses already captured FramePresentation durations.
Reports scene mean, recording mean/p95/max and render CPU-span mean. It does not
call glFinish or query the GPU. Does not include game update or presentation.
If FRAME_COST is simultaneously enabled the report explicitly says so: its total
then includes a forced wait and must not be called a pure CPU duration.

Shared common/timing_samples.h implements mean/max/nearest-rank p95 and bounded
3600-sample storage. Requested report intervals clamp to30..3600.

### 3. Optional GPU command-span and pacing reports

OPENRAC_DK_TIMING=120 enables per-slot two-record32-byte timing storage, separately
allocated as CpuUncached|GpuCached, DK_MEMBLOCK_ALIGNMENT aligned.

- First begin_list of a frame reports DkCounter_TimestampPipelineTop at offset0.
- present reports DkCounter_Timestamp at offset16 after recording its final blit
  and before final submit.
- Each record is treated as16bytes {value,timestamp}; read timestamp at offsets8
  and24. This follows the locally saved night100 backend's hardware-tested layout.
- Read only in begin_slot AFTER its existing dkFenceWait. No extra fence wait,
  glFinish or queue idle is introduced for timing.
- flush_and_wait/list splits do not restart the first timestamp: storage persists
  until the slot is reused. A frame can span several command lists.
- Conversion uses installed Deko3D0.5 public dkTimestampToNs/dkNsToTimestamp,
  defined in the SDK header. No guessed yuzu conversion constant was added.
- Invalid/sentinel/reversed/>60s timestamp spans are rejected; first four failures
  log raw values. GPU mean/p95/max reports include rejected counts.
- Also measures existing slot-fence wait, swapchain acquire wait and interval
  between successful image acquisitions. The latter is labelled present interval
  but is sampled after acquire, NOT at actual display scanout.
- Timing storage is released only after close's queue idle, before device removal.
- Allocation failure warns and disables GPU reports for that slot, not rendering.

IMPORTANT: This is GPU COMMAND SPAN, not hardware busy-time or per-section GPU
cost. Includes submission gaps, resource work and potentially frame-load stalls.
Independently verify report format/units on the Switch. A timestamp emitted on
the3D engine may not fully delimit2D/copy-engine completion; review SDK semantics
before interpreting it as whole-GPU completion. No forced Full barrier was added
just to make the measurement cover every engine. First hardware run must check
valid samples, plausible units and relationship to pacing, not assume calibration
is proved by the mock. Long slot/acquire waits are not automatically proof of
GPU saturation: scheduling and presentation also matter.

### 4. Optional audio timeline

OPENRAC_AUDIO_TRACE=1 captures at most32 stream starts in sound_engine.cpp:

- request monotonic timestamp, synchronous header duration, sector/group/queue/flags;
- reader header/body I/O duration and parse/decode duration;
- time to ready, on the reader thread after releasing the audio mutex;
- first mixer-consumption timestamp, recorded atomically in mix_streams;
- buffered/remaining-time game-side queries drain the first-consumption event and
  log it AFTER releasing the audio mutex.

No file/log I/O is added to the mixer callback. No progressive decoding or guessed
audio offset. Readiness, queuing and playback flow are unchanged. Consumption is
not first audible sound: initial silence and SDL/device output latency remain.
If a track is never queried after consumption begins, the consumption event may
not be emitted; request/read/decode logs still appear. There is no new polling
thread. Event timestamps are steady-clock milliseconds (same clock as level logs),
not game ticks; cross-reference existing frame/level logs when needed.

world.cpp use_level adds an optional RAII begin/end CPU load-span trace under the
same AUDIO_TRACE flag. End means the function returned, including its early exits,
not a claim every GPU upload has completed or that loading succeeded.

### 5. Optional panel-only trace

OPENRAC_DK_PANEL_TRACE=1:

- scene_hud.cpp draw_target_mobys logs first8 requests with tick, address range,
  framebuffer/extent/centre/clear-depth; matched actor address, model index,
  palette, lighting/blend and camera focal values.
- audit_geometry can log first32 draws into nondefault1024x1024 targets even
  while general geometry auditing is off. Reuses program/input/projection/sampler
  metadata plus metal_pass already present in the audit.

This does NOT yet log primitive-level material colours, all blend factors or
texture content/pixel hashes, and the bounded budget could be consumed by an
earlier panel before reaching the desired weapon. Actor-to-specific-material
mapping may need a follow-up trace in LevelScene::draw_layer. No shine pass has
been disabled. No claim that Pyrocitor's texture is now fixed.

## Local verification

- timing_samples_test.cpp passes mean/p95/max, empty windows and invalid samples.
- Existing real-dkgl command-stream mock passes barriers, feedback, caching and
  multi-sampler behavior. Added counter mock writes deterministic16-byte reports;
  35present calls with DK_TIMING30 produce valid1ms windows read on slot reuse,
  no mock failures. This verifies bookkeeping, not GPU timestamp hardware.
- Existing sound_engine_test.cpp was compiled natively with all audio sources,
  VAG/ADPCM/bank parsers and a lightweight log stub. Passes with AUDIO_TRACE off
  and on, including readiness, consumption, remaining duration, failed track and
  reader-stop behavior. Fixture sector200 intentionally fails to read; its warning
  is expected. Test output shows first consumption emitted from a query.
- Actual Switch compilation succeeded during implementation; final candidate
  identity/verification is recorded in last-build.json and build log. Do not rely
  on an earlier build made before final trace additions.

## Suggested first tests (when user is available)

Use build/deko3d-test/env-passive-timing.txt as a GUIDE, preserving unrelated saved
settings. No FRAME_COST. Baseline: DK_COUNTERS120, CPU_TIMING120, DK_TIMING120,
FPS1, general geometry audit0, mip audit0, draw trace0. No fallback shader/cache
switches. Same Novalis route for60–90seconds. Watch CPU/GPU timing validity and
normal FPS without forcing waits.

Separate audio run: AUDIO_TRACE1, title -> landing; capture clip and session log.
Separate vendor run: PANEL_TRACE1, visit Pyrocitor and preserve preview rotation.
Do not enable every heavy diagnostic during the performance baseline.

User is at work and cannot immediately test. No need to ask for an immediate run.

## What Claude should review or revise

1. GPU record layout, flag ordering, frame/list boundaries and slot lifetime;
   especially SDK counter behavior around2D/copy-engine work. Verify no read occurs
   before the existing fence and no timestamp storage is reset while still live.
2. Optional tracing thread behavior. Keep file I/O off the mixer thread, preserve
   locking, and improve deferred event draining only with evidence.
3. Panel targeting: add minimal material/metal-pass identification if necessary.
   Compare same-data OpenGL preview before changing actual shine semantics.
4. Pure CPU recording numbers vs GPU-span vs pacing: avoid labelling the total
   render CPU span as whole-game CPU usage or command span as GPU busy time.
5. Whether a dedicated release-library performance variant is warranted AFTER
   measurements; keep the diagnostic build and known-good NRO separately.

No PVS enablement, compression, swapchain-direct rendering, SDK workaround
replacement or shader regeneration in this pass. Preserve working geometry.

## Final built candidate

Build completed with inner exit0 and target openrac-rac1-pal_nro. NRO and ELF
copies match build outputs. All patched and stock shader stages match staged
RomFS files. ELF contains CPU/GPU timing, audio trace, panel request and optional
geometry-audit strings. Git diff whitespace check for affected tracked files is
clean. No Git commit or push.

- NRO build/deko3d-test/openrac-rac1-pal-deko3d.nro:29284749bytes.
- SHA256 a130e6695b31fbec0e400d196901c57375ed4be468d378f4a0201035a26e627d.
- Matching ELF buildID4348d6811f45cb27a088884450ccfd45c98d76cf.
- ELF SHA2561cb1c68892b4f0d51ccd65a653d81542781e2154ef7bc4bda287cf824aca1e27.
- last-build.json marks hardware_tested=false.

This is a review/measurement candidate. No claim of a measured FPS improvement
from disabling audit or of an audio/vendor correction without hardware results.

## Claude review (2026-10-29 afternoon)

See CLAUDE_REVIEW_PASSIVE_TIMING_2026-10-29.md. Four small revisions inside the optional diagnostics
(second end-of-pipe start timestamp and per-frame report trigger; audio consumption events drained
on game-thread calls; richer panel-only trace with a configurable budget). Not built yet: the NRO
above (a130e669...) does not contain them. Pre-revision sources:
build/claude-scratch/backup-pre-claude-timing-review-2026-10-29/.
