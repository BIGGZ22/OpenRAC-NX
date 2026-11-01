# Source-only optimization pass and upstream audit

User requested trying the optimization techniques from the homebrew research,
then a Claude handoff. They also asked to check upstream before making that
prompt. NO GAME BUILD: that instruction remains active. Do not run the build
scripts, change shader packs or deliver a new NRO until the user authorizes it.
Host unit tests and syntax-only checks were used below. Nothing committed/pushed.
The working tree includes substantial earlier Claude/Codex/user work; preserve it.

## Read first

- HANDOFF_2026-10-29.md and SWITCH_DEKO3D_SHADERS.md (SDK 0.5).
- DEKO3D_OPTIMIZATION_RESEARCH_2026-10-29.md (pinned reference sources).
- DKGL_STOCK_CLOCK_OPTIMIZATION_2026-10-29.md.
- DKGL_480P_TELEMETRY_2026-10-29.md.
- DKGL_SHIP_CANOPY_SOURCE_FIX_2026-10-29.md (also unbuilt).
- Earlier passive timing and transfer-sync handoffs, as needed.

## Hardware context and artifacts

Geometry now works after the UAM scheduling/driver/synchronization fixes. Keep
the patched shader pack; do not regenerate it with stock UAM. SDK 0.5 vertex
capture/replay and texture-invalidation workarounds are correctness fixes.

All earlier hardware runs were overclocked. Latest session29 started at18:19:29,
renders854x480 internally, and lacks the new vertex-cache startup message and
passive timing. Do not claim it measures the18:30 CPU optimization candidate.
Nintendo clips omit the Status Monitor overlay; capture FPS is not game FPS.
Metropolis camera rotation still has drops; cutscene audio continues while
animation can lag. Vendor Pyrocitor material and audio delay remain open.

Last built performance NRO (18:37:21) is build/deko3d-release/openrac-rac1-pal-deko3d.nro:
SHA256744afde06c059f0a9d6a43b39e0865fb6646bf4278402304e30a7aef9c492bf3,
ELF buildID00cf3165ed712b2f964a9b0ab76c308ee555bd72. It has the vertex cache,
blocking audit defaults off and revised overlay publisher, but NONE of this
pass's optimization/upstream changes or the previous canopy candidate.
Diagnostic target: build/deko3d-test, ELF buildID16c93f1c9c70d0d15b05bb81c8eb40701fc41e9b.
Matching ELF/manifests remain with each NRO. Do not overwrite them or claim they
match the current source. Telemetry connection is NOT hardware verified yet;
request overlay-status.log before claiming SaltyNX works on the user's setup.

## Implemented in this pass

### 1. CPU sampler lookup cache (default on)

dkgl_internal.h adds Context::unit_sampler_cache. Each unit remembers effective
SamplerParams, mip-level count and sampler descriptor. unit_handle still resolves
the current texture image, checks target feedback, marks sampled_epoch and calls
touch on every draw. Only the linear global sampler-descriptor search is skipped
when the key matches. No image pointer is retained by this cache. Global sampler
descriptors live for the context; they are not recycled per frame in this renderer.
Different textures with the same parameters/levels can share the sampler safely;
their image descriptors still come from the current texture.

OPENRAC_DK_SAMPLER_CACHE=0 disables descriptor-lookup reuse. State-cache reference
mode also bypasses it. Existing sampler descriptor-set refresh workaround remains.

### 2. Cached shader sampler layout

Program::sampler_uniforms retains valid sampler uniform indices. Rebuilt on link
revision or reflection uniform-count change (the latter also supports mock fixtures).
Avoids rescanning every loose uniform for every draw. Sampler units are still read
each draw, so glUniform1i changes remain visible. Duplicate binding resolution and
the existing descriptor invalidation/rebind sequence are retained.

### 3. Less palette CPU memory traffic

LevelScene::set_palette previously zeroed all palette storage then copied the
matrices over it. It now resizes retained storage, copies joints and zeros only
the padding. The texture contents and full upload extents are unchanged. Existing
unchanged-palette skip/storage reuse remain. OPENRAC_PALETTE_COPY_REFERENCE set
to any value selects the old whole-storage clear for comparison.
This does not implement partial texture upload or change skeleton ownership.

### 4. Conservative optional target barrier experiment (DEFAULT OFF)

OPENRAC_DK_LAZY_TARGET_BARRIERS=1 enables a narrow optimization: when changing
targets, skip the old target's barrier only if its writes have already been ordered
by the renderer's barrier epoch and no pending image-barrier flag exists.
Image::draw_write_epoch is stamped on clear and immediately after draw setup,
after any uniform-ring/feedback barriers, before draw emission. Both colour/depth
are marked conservatively even when write masks would exclude them.

Full copy-engine ordering and all descriptor/feedback/present barriers stay intact.
This is NOT a complete read/write hazard tracker or the full SwitchWaker algorithm.
Keep off for the first test. Review every raw command/clear/blit path and epoch
change before recommending it. No GPU pixel correctness proved by the host mock.

### 5. Counters and game-side timing

DK counters now also log cumulative sampler lookup hits/misses, layout rebuilds
and target barriers skipped. Existing vertex cache counters remain.

OPENRAC_GAME_TIMING=N (30..3600) reports mean/p95/max wall time between returning
from host vsync and entering it again. Disabled by default; no per-frame log.
Excludes host-vsync render/pacing waits, includes any game-side I/O/waits and host
callbacks outside vsync. NOT pure CPU busy time, NOT a function sampler. Use it
alongside CPU_TIMING/DK_TIMING, then profile actual hot game functions if needed.
Do not rewrite generated PS2 functions blindly or copy Bejeweled's PPC tooling.

### Existing optimizations deliberately retained

EffectRenderer already combines strips/quads into one retained vertex vector,
uploads once, merges consecutive compatible draws without reordering blending,
and reuses cached image uploads. LevelScene already skips identical palettes and
uses glTexSubImage2D within retained capacity. dkgl already has rings, heap pools,
retirement fences, uniform dirty checks and GPU skinning. Duplicating these is not
an optimization. Zcull, depth compression and tiled caching were not enabled:
those remain separate correctness-sensitive experiments, not established fixes.

## Upstream checked BEFORE writing this handoff

Fetched upstream without merging. Remote default HEAD is develop (confirmed with
ls-remote --symref), at c6adc0d7d91cb028ba660443d85bf39ac9b57e3d.
Cached upstream/master remains6360471e. New develop commits since9161f193:

- #121 /980c98cb: menu Exit placement/text IDs, save overwrite prompt and black
  loading screens. Our world_clear_colour already covers the black-screen fix.
  Other changes not imported: our Switch menu code differs and lacks mod_ui.cpp.
- #122 /93ac5584: optional fresh input and low-latency interpolation/presentation.
  NOT imported. SDL3 desktop code differs from this Switch SDL2/platform layer;
  adapt carefully, including worker ownership and settings/UI, if desired.
  This is latency work, not proven throughput improvement.
- #124 /0a9b904c: Exit/Options lifecycle changes; NOT imported for the same menu
  compatibility reason. #125 is formatting/convention cleanup, not performance.
- #126 /5d778574: language + Exit survive Quit Game. Incorporated ONLY the small
  language portion in port/game/rac1/host/options.c: after remembering a valid
  player language, update openrac_game_language too, so rebooted InitOnce sees it.
  Exit-row restoration requires the absent upstream mod_ui machinery; NOT imported.
- #119 /c6adc0d7: very large enhanced lighting/reflections/probes/water/precision
  update (~287 changed files across the compared range). NOT imported. Enhanced
  rendering is off here, and shader modifications would invalidate the precompiled
  dkgl pack. Do not mix in these files wholesale to chase a performance gain.

Evidence: git show the exact commits locally; upstream URLs are
https://github.com/OpenRAC/OpenRAC/commit/<full-or-short-hash>.
No upstream merge/cherry-pick performed; language hunk integrated manually.

## Verification performed (no game build)

- Real dkgl_core.cpp/dkgl_gl.cpp compiled into the host mocked-deko3d test.
- Zero failures: default, sampler cache disabled, lazy target barrier enabled,
  passive GPU timing enabled and panel tracing enabled (separate processes).
- New assertions: warm sampler hit, unchanged layout reuse, wrap/mip-count key
  changes, shader relink rebuild, already-ordered target barrier skip, fresh-write
  target barrier retained. Existing vertex, transfer, feedback and sampler-refresh
  regressions still pass.
- frontend geometry and overlay protocol tests pass (their binaries were from
  the preceding passes; those sources were not changed in this pass).
- Host g++ -fsyntax-only passes vsync.cpp and level_renderer.cpp. Initial vsync
  include search failed until sdk/include was supplied, then passed. No Switch
  compilation, source-to-NRO match or hardware validation claimed.
- Upstream language change not separately runtime-tested; include it in review.

Mock fixture lives in build/deko3d-debug-reference/transfer-sync-mocktest.cpp
(local build directory, not a tracked portable regression suite). Move durable
tests into an appropriate test target if continuing. Toolchain command examples
are in earlier handoffs. Do not mistake the host mock for a real GPU.

## Review priorities for Claude

1. Review correctness of the new cache and especially target-write epoch coverage.
   Inspect deletion/recreation, samplers, mip transitions, program relinks, descriptor
   lifetime, render-target feedback, command-list splits and ring reset.
2. Review source-only canopy fix: flight uses exact tick pose for camera/body/joints
   because effects have no interpolated previous vertices. It is a candidate, not
   proven hardware fix. Check packet generations/cockpit joint if exact ticks fail.
3. Confirm the small #126 language hunk fits PAL/NTSC host selection and persistence.
   Assess #121/#124/#126 Exit adaptation without importing mod UI wholesale.
4. Keep experimental barriers OFF. Plan stock-clock Metropolis rotation/cutscene
   timings with FRAME_COST off, and compare one switch at a time after build approval.
5. If game wall intervals dominate, identify hot functions with a real console
   sampler; do not claim whole-game CPU optimization based on render mock timings.

Review and revise source if justified, keep a record, run host tests. DO NOT BUILD
THE GAME until the user explicitly asks. Do not push or commit unless requested.
