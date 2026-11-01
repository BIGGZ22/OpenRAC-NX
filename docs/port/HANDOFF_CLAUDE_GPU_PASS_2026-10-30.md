# Session30-driven graphics optimization: source only

User workflow: Codex makes changes, Claude reviews/revises and returns a detailed
handoff prompt; user brings that prompt back to Codex; Codex builds at that point.
DO NOT BUILD NOW. Claude must not build either. No shader-pack regeneration,
commits, pushes or external messages. Preserve the large uncommitted working tree.

## Context and evidence

Read STATUS_MONITOR_HARDWARE_SESSION30_2026-10-30.md and the preceding Claude
review/build notes. Status Monitor1.4.1+r4 is confirmed to display FPS/resolution:
1280x720 output,640x480 internal. Sampled phone-video frames show16.5–20.2rolling
FPS, GPU77.8–89.5% at307.2MHz, CPU around1022MHz with the busiest game core near40%.
These are snapshots, not whole-run averages. They support substantial graphics
pressure, not CPU saturation. Game logs show up to roughly2450draws/frame and
hundreds of sampler-set refreshes, large GPU command spans and slot waits.
Command span is not pure GPU busy time. FRAME_COST was off. See session30 inputs
and extracted frames in build/deko3d-test/session30-review/.

Last built performance NRO is still the01:05 build:
SHA25654184009d5ccf59b45c9d8ba0013726f3e24c991bf7488b127f129374747acb3,
ELF buildID3a507cc6654328f3700f8a5a6f30e2a0dbfd4ffc.
Neither that NRO nor the diagnostic one contains this source pass. Matching ELF
and manifests must stay with them. No game build ran during this pass.

## Changes

### 1. Skip provably empty glow groups

Files: scene/moby_passes.h, scene/level_renderer.h/.cpp.

mesh.vert initializes v_glow=0. It can set it to1 only when abs(moby_light.w)>0.5,
moby_ambient.w>0.5 and the vertex is a glow vertex. mesh.frag discards non-glow
fragments in either glow pass. Existing primitive-level glow skipping knows only
whether vertices could glow, not whether any live instance enables glow.

New moby_instance_may_glow(light_mode,glow_word) implements those instance gates.
Non-finite inputs return true conservatively. Group::may_glow is recalculated
from ALL uploaded instance records in fill_group, covering initial groups,
pooled groups and updated live groups. A group is skipped in both glow steps
only when NO instance can glow. Other passes and mixed-instance groups retain
their draws. Material/texture coverage, shader files and draw order unchanged.

OPENRAC_MOBY_EMPTY_GLOW_SKIP=0 restores the old group submission; default on.
Review against actual shipped shader semantics, live colour writes, UI instances,
group refresh paths and mixed groups. Host helper tests alone do not establish
scene equivalence; add an integration fixture if practical.

### 2. Batch adjacent changed texture bindings

Files: renderer/dkgl/dkgl_gl.cpp, dkgl.h, dkgl_core.cpp.

prepare_draw still resolves every required sampler, marks image reads/touches,
performs feedback checks and refreshes the sampler descriptor set as before.
It computes a dirty-slot mask from the existing sent-state cache, then emits
consecutive changed bindings in one dkCmdBufBindTextures call per shader stage.
Gaps are not overwritten. Vertex and fragment stages still receive identical
handles. Warm unchanged draws still emit no texture binding commands.

OPENRAC_DK_TEXTURE_BIND_RUNS=0 emits each changed slot separately, as before.
Default on. Counts texture_bind_calls and texture_bind_slots are logged as totals
when DK_COUNTERS runs (both stages counted). This reduces command/API overhead;
it DOES NOT reduce actual sampler-set refresh frequency. Do not claim it does.

### 3. Optional per-pass geometry counts

OPENRAC_SCENE_PASS_COUNTS=N (30..3600) samples moby draw_layer calls whose input
frame is divisible byN; default off. Logs6step arrays of draw calls and submitted
instanced triangles, plus group skips in the two glow steps. Order:
solid, glow-depth, glow-blend, translucent, effects, metal.
These are snapshots per draw_layer call, not interval means or GPU timings.
Vendor/offscreen calls or intermediate display frames can produce multiple lines
with the same input frame. Skipped-group count is before visibility/blend checks,
so it is an eligibility count, not measured saved draws. Do not call it FPS gain.

## Verification (PC only)

- moby_passes_test compiled and passed: zero glow flags, static lighting, threshold
  equality, live/scripted/UI lighting, enabled glow, conservative NaN handling,
  and prior depth-order/reference assertions.
- level_renderer.cpp host g++ -fsyntax-only passed against current generated headers.
- Real dkgl_core/gl compiled in the existing mocked SDK fixture. Zero failures:
  default, texture-bind runs disabled, lazy barriers enabled, passive timing,
  sampler cache disabled, and panel tracing (separate processes).
- New fixture captures actual texture-binding commands: adjacent slots5/6 and
  disjoint slot8, both shader stages, exact handles and untouched gap7. Default
  emits4calls versus reference6 for the same6stage-slots. A warm repeat emits0.
- Existing transfer, feedback, sampler refresh, cache and Claude's lazy barrier
  regressions continue to pass.

Mock test source: build/deko3d-debug-reference/transfer-sync-mocktest.cpp.
It is a local fixture, not currently a portable tracked test target. Move durable
regressions into the repository test suite when appropriate. GPU pixel correctness
and hardware performance remain unverified. No full Switch compile/link performed.

Backups of pre-pass renderer files:
build/claude-scratch/backup-pre-codex-gpu-pass-2026-10-30/.

## Protected behavior

Keep SDK0.5 vertex capture/replay workaround, patched UAM pack, texture-cache
invalidation mapping, Full copy-engine barriers and the sampler-set refresh
workaround. Lazy target barriers remain OFF by default. Keep the reviewed canopy
world capture, saves, language persistence and launcher. The canopy fix activated
in session30; that log does not prove visual attachment. Audio delay unresolved.
Do not interpret low GPU-resolution scaling as evidence that CPU alone limits.

## Claude tasks

1. Read this and CLAUDE_REVIEW_OPTIMIZATION_2026-10-29.md; inspect actual source.
2. Review/revise the two optimizations, especially proving a skipped glow group
   produces no colour/depth output, and correct flags after pooled/live updates.
3. Review batching against SDK0.5 bindings, program switches, resets, duplicate
   bindings, non-contiguous slots, both stages and descriptor visibility.
4. Investigate remaining actual GPU work from source/available logs. Propose
   or implement further conservative changes with reference switches. Do not
   simply disable correctness workarounds or reduce visual quality silently.
5. Run host tests and syntax checks; do not build the game or regenerate shaders.
6. Save a dated review with exact files changed, reasons, tests, limitations and
   a baseline/one-switch-at-a-time hardware plan.
7. Return a COPY-AND-PASTE PROMPT FOR CODEX explaining what you changed, what to
   review/fix next, and how to build/test after this handoff returns to Codex.
   Explicitly state that you did not build. Codex will build after the user brings
   back your handoff. Include any blockers, unresolved risks and build instructions.

When Codex later builds: preserve prior NRO/ELF/manifests, use the installed
Switch toolchain, verify inner exit code0 plus NRO target, compare copied NRO/ELF
hashes and staged shader pack hashes, record new build IDs/manifests. Primary
performance script claude-build-deko3d-release.sh; diagnostic claude-build-deko3d.sh.
Never infer success from the scripts' outer exit alone. Initial hardware test:
stock clocks, explicit640x480 (same aspect as session30), same FPS cap/route,
passive timing plus GAME_TIMING/DK_COUNTERS/SCENE_PASS_COUNTS120, FRAME_COST absent,
lazy barriers0; compare Metropolis rotation and ship flight/landing.
