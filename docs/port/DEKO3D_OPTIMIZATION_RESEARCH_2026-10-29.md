# Homebrew optimization review (research only; no build)

Reviewed online sources and current renderer source on 2026-10-29. No OpenRAC
runtime code changed in this research pass. The source-only flight-canopy
candidate from the previous turn remains unbuilt. No game build was run.

Fresh source snapshots are in build/graphics-references/optimization-review-2026-10-29/:

| Project | Reviewed commit |
| --- | --- |
| NetherSX2_nx | a78cb5836adb9024e6b0cd9b5860a526d4dda68b |
| SwitchWakerHD | b2e51df20d5a763e85bf77602c0d406d799872ee |
| night100 | d97ba7780e5d46f70136087009948a1050f3d9e0 |

SwitchWaker migration notes and Bejeweled optimization documentation were also
opened online; earlier locally saved renderer examples remain in graphics-references.
Project-specific measured gains below are reports by their authors, not OpenRAC gains.

## Findings and actual applicability

### SwitchWakerHD: cache the work before emitting GPU bindings

Source: runtime/src/gfx/deko/draw.cpp. sync_cache keys state by epoch;
texture entries track surface and texture epochs, descriptor contents and
alias write stamps. bind_runs compares slots and binds consecutive changed
slots in runs. This avoids both redundant commands and repeated resource lookup.

OpenRAC already skips many unchanged GPU bindings, caches uniform uploads and
now caches CPU vertex reconstruction. However prepare_draw still scans sampler
reflection, resolves texture/sampler handles, initializes several temporary arrays
and performs lookup work on repeated draws. A generation-keyed sampler-layout
and texture-handle fast path is a concrete next CPU optimization candidate.
Keep per-draw resource touches, feedback checks and the sampler-rebind workaround:
skipping them along with lookup would be a correctness regression.

Source link:
https://github.com/centollOS/SwitchWakerHD/blob/b2e51df20d5a763e85bf77602c0d406d799872ee/runtime/src/gfx/deko/draw.cpp

### SwitchWakerHD / NetherSX2: track hazards instead of global waiting

SwitchWakerHD docs/deko3d-plan.md describes per-surface read/write epochs and
upload batching. Its traced examples report 47 to 20 fragment barriers, not a
general FPS multiplier. Copy-engine transfers keep Full barriers. Tiled cache
and depth compression are explicitly experimental/off by default in these notes.

NetherSX2 source/deko3d/dk_image_sync.h stamps image writes and advances an epoch
when a fragment/image invalidation makes those writes visible. dk_gpu.cpp tests
this before sampling. It also deduplicates descriptors and recycles per-submission
upload chunks after that submission's fence completes.

OpenRAC already batches pending copy work, tracks full epochs and waits before
reusing frame slots. Do not redo these as if absent. There may still be room to
reduce conservative image barriers, but first count barriers by reason. Its earlier
transfer-ordering bug corrupted geometry/textures: a 3D-only barrier must never
replace the Full barrier needed to order the separate copy engine.

Sources:
https://github.com/centollOS/SwitchWakerHD/blob/b2e51df20d5a763e85bf77602c0d406d799872ee/docs/deko3d-plan.md
https://github.com/NaGaa95/NetherSX2_nx/blob/a78cb5836adb9024e6b0cd9b5860a526d4dda68b/source/deko3d/dk_image_sync.h
https://github.com/NaGaa95/NetherSX2_nx/blob/a78cb5836adb9024e6b0cd9b5860a526d4dda68b/source/deko3d/dk_gpu.cpp

### night100: immutable pooled resources and less particle upload work

src/graphics/deko3d_backend.cpp retains resources until a command slot's fence
completes; immutable render-texture snapshots are reused from a bounded pool.
Tests exercise ordered snapshots and image reuse. Native renderer uniform values
are cached. Boneless particle revisions avoid uploading an unused 4 KiB palette.
The README also lists GPU skinning, native particles, separate IOP work and clock
boost. Those reported results are not a stock-clock OpenRAC comparison.

OpenRAC already skins on the GPU and owns bounded render snapshots. Inspect
instance/palette/particle upload bytes and transient allocations before adding
another pool. Where particle work is significant, batch compatible particles
and avoid unnecessary palette uploads, preserving blend/depth ordering. Moving
draw submission off-thread is not automatically new: OpenRAC already has a
bounded RenderWorker. More queued frames can increase latency and memory use.

Sources:
https://github.com/matthewperiut/night100/blob/d97ba7780e5d46f70136087009948a1050f3d9e0/src/graphics/deko3d_backend.cpp
https://github.com/matthewperiut/night100/blob/d97ba7780e5d46f70136087009948a1050f3d9e0/src/graphics/native_renderer.cpp

### SwitchWaker: offline shaders, cached state and measured GPU changes

The migration notes describe precompiled shaders, cached pipeline/state handling,
streaming rings, and later GPU-time-driven resolution. Much of the shader/precompiled
work already exists in dkgl. Lower resolution helps only when pixel work is costly;
it does not remove game simulation or CPU command-recording work.

Source: https://github.com/centollOS/SwitchWaker/blob/main/docs/DEKO3D_MIGRATION_PLAN.md

### Bejeweled3: optimize generated game code from console profiles

docs/optimising-functions.md starts from A57 console sampling, identifies specific
hot functions, reduces register-context loads/stores and calls, and checks identical
guest memory/register/float behavior. This is a CPU recompiler technique, not a
Deko3D graphics switch. Its PPC tools cannot be applied wholesale to PS2/MIPS.

Potentially important for OpenRAC if the game tick, rather than recording/GPU work,
dominates at 480p. Current passive CPU renderer timings do not measure the entire
game tick. Profile it separately before rewriting generated math or effect updates.

Source: https://github.com/ChanseyIsTheBest/bejeweled3-nx/blob/main/docs/optimising-functions.md

## Suggested order for OpenRAC

1. Measure the latest release candidate at stock clocks: game tick, scene capture,
   record CPU, GPU command span, slot/acquire waits and cache hit rates. Session29
   was 480p but appears to predate the new CPU-cache build; no passive timings.
2. Add a CPU sampler/texture binding fast path, preserving hazard/lifetime checks.
   Test texture replacement, deletion/name reuse, sampler changes, RTT feedback,
   program changes, descriptor resets and ring resets.
3. Measure dynamic instance/palette bytes, allocations, draw counts by pass and
   particle counts. Reuse scratch storage or batch only where those numbers justify it.
4. If GPU-bound, experiment separately with hazard-based barrier reduction. Re-run
   transfer, palette, feedback, mip and real scene tests. Keep rollback switches.
5. If game-tick-bound, profile generated CPU functions and optimize identified hot
   paths with equivalence checks. Reducing resolution cannot solve those costs.

Do not blindly enable Zcull, depth compression or tiled caching: dkgl previously
had correctness trouble in these areas. Do not remove Full transfer barriers,
rewrite the patched shader pack, hide slow frames in FPS reporting, or promise
another port's percentage gains. No new stock-clock improvement is proven here.
