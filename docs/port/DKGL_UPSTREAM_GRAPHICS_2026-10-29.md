# Graphics-focused upstream integration, October 29

## Hardware evidence before this candidate

Session openrac-rac1-pal (21).log passes all seventeen GPU checks, including
culling and four independently rotated weighted joints. Sampled vertex/index
and instance fetches remain within storage. Supplied clips still show malformed
models and white scenery triangles. This rules out no more than the checked
operations; it does not establish correct geometry for the whole game.
Driver (8) has driver (7) as an exact prefix and adds only two queue creations;
its earlier method-error records are historical.

Artifacts, logs and previous shader pack were archived before replacement in
build/deko3d-test/diagnostics/2026-10-29-full-audit/.

## Integrated code

Fetched upstream/develop at 9161f193. Integrated applicable graphics/gameplay
parts of these upstream commits into the existing Switch tree, preserving
Switch FOV and native renderer changes:

- fb740d63: zero initialization of generated local scalar/aggregate variables.
  Regenerated and recompiled PAL game code. Upstream identifies undefined local
  reads as the cause of missing drones and mispositioned Novalis sparks. The
  larger moby-pool toggle is included with its regression; the default remains on.
- 24d97134: unusable camera pose/matrix guard, last-good-camera retention and
  bounded rejection logging. World packet and background decisions use the
  guarded camera result.
- 9954a3bd: game's level background colour, retained world background, and sky
  sprite texture overrides using the existing level override path.

User then prioritized graphics over mods. New mod interfaces, desktop/Android
features, Exit-menu changes and unrelated co-op glove update #120 are excluded
from this batch. The old PAL camera-return and item-pose host replacements are
retained: wholesale deletion would assume decompilation updates absent here.

Sources:
https://github.com/OpenRAC/OpenRAC/commit/fb740d63b0d28da895c3941b1fffabdc762aec28
https://github.com/OpenRAC/OpenRAC/commit/24d9713408354b1cf123b649f8c502262bcc4184
https://github.com/OpenRAC/OpenRAC/commit/9954a3bd

## Shader candidate and packaging correction

New dk_rebuild.py rebuilds each stage from the SPIR-V embedded in its existing
.prog, with explicit GLSL 460 and vertex-Y-flip options and installed SDK UAM.
All eighteen stages compile; seventeen binaries differ from the old pack. A
binary difference is not evidence of a compiler bug or a visible correction.
The nine .prog reflection files remain unchanged. SHA256 provenance records tool
executables, intermediate SPIR-V/GLSL and old/new DKSH in pack/provenance.json.

CMake previously copied the pack only on configuration. GLOB CONFIGURE_DEPENDS
detects name changes but not modifications to existing shader bytes. Explicit
CMAKE_CONFIGURE_DEPENDS now requests configuration when pack contents change.
All eighteen copied shader hashes match the source pack. Runtime logs include
loaded stage byte counts and FNV fingerprints, permitting exact identification
even when shader-only changes do not alter the ELF build ID.

## Verification

- Successful Switch build, exit code zero; copied NRO/ELF hashes match outputs.
- Build ID f1608851276abe6455abbbcde98dbd3c3a30679a; manifest last-build.json.
- Native camera geometry regressions pass, including invalid/degenerate views.
- Native larger-pool toggle regression passes.
- Upstream zero-init translation/compile/run regression passes with a temporary
  Windows VirtualAlloc adapter for its POSIX-only mmap harness. The stock test
  skips on Windows; MSYS Clang lacks sys/mman.h. Adapter and runner are local
  debugging artifacts; the production generator and upstream assertion are used.

Candidate: build/deko3d-test/openrac-rac1-pal-deko3d.nro.
Detailed geometry audit and seventeen automatic hardware checks remain enabled.
No visible model/scenery correction is confirmed until a new Switch run.
