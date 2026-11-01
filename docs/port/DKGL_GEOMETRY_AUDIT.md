# Deko3D geometry investigation: combined test

The user confirms malformed models persist after all twelve initial GPU checks
pass. Pyrocitor texture is visible, but its geometry remains incorrect. Neither
passing these checks nor compiling successfully establishes visual correctness.

## Changes

- Seventeen automatic pixel checks, including front/back face classification
  under both winding conventions and four independently rotated weighted joints.
- Every indexed draw checks the complete index byte range before submission.
- Buffer updates use subtraction/division bounds checks, avoiding signed
  offset-plus-size overflow and reporting rejected updates.
- Bounded real-draw audits scan indices, handle primitive restart, and check
  reflected enabled vertex/instance inputs against the actual buffer sizes.
- Audit records shader, source buffer versions, index range, instance count,
  sampled positions/matrix columns/weights/palette base, finite values,
  projection matrix, skinning mode, sampler units/image dimensions, culling,
  blending, depth comparison and target size.

Sampling runs every sixty renderer frames, at most sixteen draws per sampled
frame and 4096 snapshots per process. Each program/skinning-mode combination
gets a sample before rotating through additional draws. This prevents early
terrain draws from excluding weapon and character data. Logging is diagnostic
work: do not compare performance against a build without this audit.

Full index-range guards run on every indexed draw. Full vertex fetch validation
is sampled, not exhaustive. Nonfinite checks cover sampled float inputs, not
every vertex or GPU-produced position. Palette matrices are not mirrored back
from GPU memory. No shader or winding behavior is changed by this batch.

## Verification and one combined hardware run

Native geometry regression covers exact end-of-buffer fetches, overflow counts,
zero stride, index offsets, 16-bit restart sentinels and invalid index widths.
It and the four existing native dkgl regression executables pass. The Switch
build completes and copied NRO/ELF hashes match the build outputs. Identity is in
build/deko3d-test/last-build.json.

Use build/deko3d-test/openrac-rac1-pal-deko3d.nro. Leave
OPENRAC_DK_DIAGNOSTIC absent or none. No extra audit setting is required.
Visit the title model, landing cutscene, Novalis scenery and vendor weapon
preview in one session. Collect the session log, driver log and startup log,
with a clip of the remaining visible defects. Session log contains both
`dkgl selftest` and `dkgl geometry` entries. Hardware confirmation remains
necessary before claiming that any visible model is correct.
