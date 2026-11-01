# CPU vertex setup cache and stock-clock baseline

The user clarified that all previous hardware runs used an overclocked Switch.
Do not describe previous FPS or CPU/GPU timings as stock-clock results. The
latest hardware log, openrac-rac1-pal (28).log, confirms an internal 1280x720
frame and picture, one sample. Auto selected 720p. The user is trying explicit
480p next; no stock-clock results for this optimization are available yet.

## Changes

The existing GPU state cache avoided resending identical vertex bindings, but
still rebuilt the attribute, stride and address arrays on the CPU each draw.
The new cache skips that reconstruction when the input state is unchanged.

Its key includes VAO identity/revision, program identity/link revision,
reflected input count, frame-storage epoch, current GPU-state vertex count
and every participating buffer's content version. Buffer/VAO deletion and
command-state reset invalidate it. CPU-skinning overrides take the original
path. Changing buffer contents, attribute pointers, divisors, shader links
or frame storage forces reconstruction. Reapplying identical attribute
settings does not invalidate the cache. No shaders or geometry formats changed.

Enabled by default. For an A/B test only, OPENRAC_DK_VERTEX_CACHE=0 restores
CPU reconstruction. Existing OPENRAC_DK_STATE_CACHE=0 also bypasses eligibility
through the existing state-cache switch. Counters include setup hits/misses.

Sources: port/renderer/dkgl/dkgl_gl.cpp, dkgl_core.cpp, dkgl_internal.h, dkgl.h.
Pre-change source and prior working NRO/ELF/manifest are backed up under
build/deko3d-test/diagnostics/2026-10-29-before-stock-clock-optimization/.

## Verification

The real renderer compiled against the local mock SDK and passed with zero
failures in default, passive timing, panel trace and new-cache-disabled modes.
New checks cover repeated inputs, redundant setters, buffer-content changes,
pointer changes, shader relinking, storage epoch changes and state reset.
Fixture: build/deko3d-debug-reference/transfer-sync-mocktest.cpp.

An x86 mock benchmark of 200,000 draws measured about 0.169 us/draw with the
cache versus 0.297 us/draw without it. This measures host CPU setup with GPU
calls mocked; it is not a Switch FPS estimate. Real benefit depends on hit rate.

Both Switch builds finished with inner exit code 0 and the NRO target built.
Copied NRO/ELF hashes match their build outputs. All patched and stock shader
pack files match their respective staged RomFS directories. Final incremental
builds include the redundant-setter change. Separate last-build.json files
record hashes and hardware_tested=false.

| Variant | NRO SHA256 | ELF build ID |
| --- | --- | --- |
| Diagnostic, build/deko3d-test | 0bbe03f310fa0ede7d379447d3e540ca4df42bd6e482a83f2b0b8b107b4b84b8 | 358d5fbd25615f87d7a15ff9fce310ea47d60edd |
| Performance, build/deko3d-release | ce51ce85b058b61a305509d3fdb742a3bcd640a77633caeab7f139017d40d275 | f5168d843a23917af184c113947cc6e6ee07b055 |

The existing claude-build-deko3d-release.sh builds the separate performance
target using libdeko3d.a. The diagnostic target links libdeko3dd.a. Release
removes SDK validation overhead but also removes useful driver checks; use
the diagnostic version if a rendering regression appears. The release-library
benefit is not measured on hardware yet. Both contain the same game/cache work.

## Next hardware test

1. Keep the previous working NRO. Copy the performance NRO to the normal game
   location. Keep its matching ELF/manifest on the PC for crash decoding.
2. Select explicit 480p in the launcher. Use stock clocks, the same docked or
   handheld mode, the same FPS cap and the same Novalis route each comparison.
   This change does not alter resolution defaults or clock settings.
3. Leave OPENRAC_FRAME_COST off. For a clean play test env.txt is optional.
   For measurements use the existing env-passive-timing.txt settings instead;
   CPU/GPU timing is nonblocking but GPU span is not pure busy time.
4. Collect the game log and note FPS, visual regressions and actual clocks.
   If comparing 720p, change only resolution. If comparing cache on/off, change
   only OPENRAC_DK_VERTEX_CACHE and restart. Preserve each run's logs separately.

Lower resolution reduces pixel work; CPU draw submission may remain limiting.
The pale Pyrocitor vendor preview and landing-scene audio delay are still open.
No fix for those is claimed here. Keep the working geometry/shader fixes.
No commits or pushes were made.
