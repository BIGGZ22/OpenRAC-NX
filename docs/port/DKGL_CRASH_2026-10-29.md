# Novalis GPU/cleanup crash investigation — 2026-10-29

The first offset guard did not trigger in the supplied session log. The driver
log is cumulative; its newest failure writes `0xff000000` to Maxwell method
`0x739` (VertexArray[14].Start high word). Earlier failures write `0xc1df6200`.
The session still reports zero missing programs.

The supplied Atmosphere report's module ID matches the preserved ELF build ID
`d74a1c09781420b4d26e685f7f00223a2921119c`. Addr2line maps its PC to `_free_r`,
then `dkgl::close()` at the texture-table clear (dkgl_core.cpp:728 in that build).
This is cleanup after the GPU error, not evidence that TexturePool itself caused
it. Heap corruption is plausible but its origin was not yet established by the
stack alone.

## Verified SDK defect

Pinned [Deko3D v0.5.0 gpu_3d_vbo.cpp](https://github.com/devkitPro/deko3d/blob/v0.5.0/source/maxwell/gpu_3d_vbo.cpp)
reserves `5*numBuffers` words in `dkCmdBufBindVtxBuffers` but emits two three-word
packets per binding: six words. CmdBufWriter's normal stream operator does not
reserve again. Disassembly of the installed libdeko3dd.a confirms both the 5*n
reservation and 24-byte (six-word) per-binding writes. Near the end of a command
chunk, these writes can overrun the reserved space. This is a concrete defect,
not yet hardware proof of the sole cause of this particular crash.

## Local workaround

`vertex_bind_compat.h` captures the SDK's own binding commands into an adequately
sized stack buffer using a dedicated CPU-only scratch command buffer. It checks
word count and a canary, then calls public `dkCmdBufReplayCmds`, which reserves
the actual six-word count before copying. The live rendering path no longer calls
the defective binder directly. The installed SDK is unchanged. Capture failure
sets a sticky renderer error. The startup session log identifies the workaround.
The earlier offset guard remains. No model/shader/texture visual fixes are bundled.

The SDK's vertex-buffer-state function explicitly disables slots beyond its
count, so the stale-enabled-slot theory is not supported by this SDK source.

## Validation and next run

The host regression models the verified SDK encoder and checks every legal
first/count range, empty buffers, command-chunk boundary reservation, out-of-range
inputs and capture failure. It passes with -Wall -Wextra -Werror. This is a host
model, not execution of the AArch64 driver. The game NRO builds successfully via
the existing build script; copied NRO/ELF bytes, headers and workaround message
were verified. Output fingerprints are in `build/deko3d-test/last-build.json`.

The previous NRO/ELF and all five supplied logs were preserved under
`build/deko3d-test/diagnostics/2026-10-29-guard-crash/` before rebuilding.

Test Novalis for 3–5 minutes with the same settings and return fresh driver,
runtime, session and any Atmosphere crash logs. Check the startup session for
`SDK 0.5 vertex binding capture/replay workaround enabled`. Other visual bugs
(ship, menu Ratchet, white shards, vendor preview) remain separate investigations.

## First hardware retest after workaround

The user reports no crashes. Session `2026-10-29 01:32:39 -0400` explicitly logs
the capture/replay workaround as enabled, contains no `[error]` lines and continues
through measured tick 6550, beyond the previous failure around tick 2302. No
missing shader programs are reported. The cumulative driver file exactly extends
the previous file with one queue-creation diagnostic and no new GPU error; the
runtime file is byte-identical to the old file. Old queue errors in those files
are not new failures. This is evidence for the fix in this run, not long-term
stability proof.

Reviewed sampled frames from both supplied ~30-second 1280x720/30fps recordings:
`2026102901334500-03DB12780BD84000.mp4` and
`2026102901342600-03DB12780BD84000.mp4`. Large white/light-blue ground patches and
dark angular strips across walkway surfaces remain visible. Captured imagery
alone does not determine whether their cause is culling, depth, sampling or
geometry. Do not describe the whole rendering issue as fixed or infer game FPS
from a 30fps capture. These clips do not show the landing ship or main-menu
Ratchet/vendor previews, so those reports cannot be reassessed from these clips.

Post-workaround logs and the tested build manifest are preserved under
`build/deko3d-test/diagnostics/2026-10-29-post-workaround/`; sampled clip contact
sheets are in `build/deko3d-test/clip-review-2026-10-29/`. No new executable was
built in response to this review. Next priority is diagnosing visual draw-state
and vertex/texture correctness while retaining the crash workaround.

## Graphics retest at 01:50:25

The next session explicitly enables both winding/alpha corrections. The user
reports the visuals look unchanged; the latest cutscene recording visibly still
has a malformed landing ship. Do not mark those visual bugs fixed. The supplied
01:35:02 clip is from the earlier run, whereas the other three clips are from
01:50–01:51. Keep earlier white-patch imagery separate from latest-build evidence.
All supplied logs and the NRO/ELF for this round were archived under
`build/deko3d-test/diagnostics/2026-10-29-graphics-round1/`.

Added opt-in OPENRAC_DK_DIAGNOSTIC modes to isolate the next issue:
- bind-pose: overwrite only the GPU snapshot's reflected int skinning uniform
  with zero, for mesh and depth variants; retain CPU uniform state and all draws.
- no-effects: omit only programs matching the actual effect vertex interface.
- no-shadows: omit only the opacity-uniform shadow volume/composite pair.
- none/absent: unchanged normal rendering. Invalid values warn and disable modes.

Diagnostic omissions and active mode are logged. The shader classification and
safe override tests use the actual source shader interfaces, verify unrelated
programs remain active, and reject out-of-range uniform writes. These modes are
intentional picture changes for investigation, not fixes or performance claims.
First test the landing ship in bind-pose mode, then choose the next change from
that result. The crash workaround remains unchanged; no animation assets or game
save behavior are modified. No new shader pack compilation is required.
