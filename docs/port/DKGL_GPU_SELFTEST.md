# dkgl real-shader GPU checks — 2026-10-29

The latest bind-pose session explicitly enables that mode for the mesh/depth
programs. The video changes geometry, but a contact sheet alone does not establish
whether all missing ship parts return. Do not claim a definitive palette failure
from this result.

Downloaded and built the official KhronosGroup/SPIRV-Cross inspection tool in
`build/deko3d-debug-reference/`. Audited the 9 shipped .prog files against its
reflection: 133 uniform/sampler checks, 67 vertex attribute columns (including
expanded mat4 columns), and 33 linked varyings have no mismatch. JSON reports and
converted GLSL are in `build/deko3d-debug-reference/shader-audit/`. The installed
SDK UAM recompiles a mesh vertex shader, but its output differs from the earlier
pack. Different tools/code generation alone is not proof of a defective pack;
the shipped shaders were not replaced on that basis.

Official reference: https://github.com/devkitPro/uam/blob/master/README.md
requires explicit resources and treats stages as separable. The GLSL pack uses
explicit stage input/output locations and per-stage sampler bindings. The
reflection audit checks actual inputs and outputs, not guesses from screenshots.

## New automatic hardware checks

`scene/dkgl_selftest.cpp` runs once after the real mesh shader is built on the
Deko3D Switch path. It draws into a temporary 16x16 framebuffer using the actual
mesh shader, the native 120-byte instance layout and the scene vertex layout:

1. Nonidentity instanced model matrix and an index-buffer byte offset.
2. RGBA32F joint texture fetch with unsigned-byte integer joint index 1.
3. Updating that joint texture moves the triangle outside the target.
4. A second texture update returns the expected red pixel.
5. A palette base of 256 crosses into row 1 of a 1024-wide float texture.

These checks exercise the whole dkgl upload/binding/shader/raster/readback route,
not a CPU model. Each check logs its name, actual RGBA pixel, GL error and PASS or
FAIL. A summary is in the normal session log. Failed pixels do not deliberately
abort gameplay. The small temporary GPU objects are deleted afterward; subsequent
scene upload/frame setup establishes normal draw state.

The check uses the already shipped shader pack; it needs no new shader program.
It is compiled only for the Deko3D Switch game target. OpenGL and Vulkan codepaths
are unchanged. When OPENRAC_DK_DIAGNOSTIC is active the check logs a skip, to avoid
bind-pose or pass omission invalidating the expectations. Remove that environment
line and boot normally to run it. Running the tests on hardware is still pending.

Build via claude-build-deko3d.sh completed with exit code 0. The copied NRO and
matching ELF were byte-verified and contain the test strings. The build manifest
is `build/deko3d-test/last-build.json`. Prior test artifacts and the supplied
session/driver/startup logs are preserved in
`build/deko3d-test/diagnostics/2026-10-29-bindpose/`.

Next evidence needed: boot to title and return
`sdmc:/switch/openrac/data/openrac-rac1-pal.log`. Find `dkgl selftest` entries.
No visible graphics fix is claimed by adding these checks.

## Hardware result and queued-draw follow-up

The 02:25:35 session on October 29 passes all five checks with GL error zero.
Matching artifacts and all three supplied logs are archived under
build/deko3d-test/diagnostics/2026-10-29-gpu-checks/.
The original tests read pixels after every draw, waiting for the GPU. That can
hide resource lifetime and synchronization failures between queued draws.

The follow-up adds seven assertions: old/new joint-texture contents in separate
scissor regions; old/new instance-buffer and uniform contents; reversed-depth
occlusion; and two simultaneous instances. Total: twelve checks. No intermediate
readback separates the paired draws. Source already declares invariant
 gl_Position; GL_EQUAL applies conditionally to reused world prepass depth, so
there is no evidence yet to justify changing the game's depth comparisons.

The follow-up build succeeded; copied NRO and ELF match their build outputs.
Build ID: 143433712924d6935ae8c41f2be021989943737c.
The seven added assertions await hardware execution. This is a diagnostic build,
with no claimed visual correction. Boot normally and collect the session log.
## Queued checks hardware result

The next supplied session, openrac-rac1-pal (20).log, passes all twelve checks
with GL error zero, then continues through Novalis to tick 5700. No missing
programs or diagnostic omissions are reported. The driver (7) log contains the
entire previous driver (6) log as an exact prefix, followed only by one queue
creation line: the old method-error entries are historical, not a new failure.
The recorded gameplay resolution is 854x480. This is not evidence that the
visible model/scenery defects are fixed; no new video was supplied this round.
Artifacts and supplied logs are archived in diagnostics/2026-10-29-queued-checks/.
Basic instance stepping, queued palette updates, queued instance/uniform updates,
and reversed-depth occlusion now have passing hardware evidence. Remaining
investigation should target actual failing geometry, shader math and pass state,
rather than repeat these isolated checks or infer a fix from counters alone.

## Visible defects persist; culling and weighted joints

User confirms the models remain broken after the twelve passing checks. Pyrocitor
texture now appears to load, but its model is malformed. Added four face-culling
assertions for GL_CCW/GL_CW and GL_FRONT/GL_BACK using a known CCW triangle; the
prior twelve checks all disabled culling. Added a four-joint weighted blend with
independent nonidentity rotations and compensating translations. Total seventeen
assertions. These new assertions await hardware results. Build succeeded and
copied NRO/ELF hashes match their build outputs. See last-build.json for identity.
No visible model fix is claimed.
