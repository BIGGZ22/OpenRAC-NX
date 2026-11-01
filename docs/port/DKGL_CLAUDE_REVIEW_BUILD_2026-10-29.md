# Review and build of Claude's overnight changes

User authorized a new build after asking to review Claude's work. Reviewed
HANDOFF_CLAUDE_2026-10-29_TRANSFER_SYNC.md in full, SWITCH_DEKO3D_PORTS_NOTES.md,
OTHER_RAC1_PORTS.md and relevant native renderer/compiler/packaging code.

## Understanding of the changes

- Explicit Full barriers order copy-engine transfers against rendering. Previous
  hardware results in Claude's notes pass the transfer/mipmap checks but show no
  visible correction. CPU-skinning/palette checks also did not change the picture.
- Latest shader pack applies three UAM scheduling patches: initialize scheduling
  metadata, disable dual issue and stall across block boundaries. This is a new
  compiler experiment compared with the earlier stock-UAM rebuild. It preserves
  shader reflection and instruction semantics while changing scheduling controls.
- Fragment/tile texture invalidation uses a wait-for-idle path to avoid the SDK's
  suspect waiting-invalidation method. First-use samplers trigger descriptor-set
  rebinding. Both are enabled by default, with comparison switches retained.
- Zcull is off by default; enabled clip distances now follow GL state. These aim
  to prevent incorrect surface rejection. Their hardware benefit is unconfirmed.
- Draw-state caching avoids repeated bindings. Same-size colour copies normally
  use the 2D engine. Self-target sampling now uses the actual colour image plus a
  barrier; this remains a correctness risk to observe in the vendor/menu targets.

This review does not independently establish every source claim in Claude's
notes. The build and source inspection are not a substitute for hardware results.
No claim is made that the model corruption is solved.

## Build verification

Ran claude-build-deko3d.sh through local MSYS2. Successful inner exit code0,
openrac-rac1-pal_nro completed, outputs copied. Kept diagnostic libdeko3dd.a
linked. Did not run the separate release script or modify renderer source.

- NRO: build/deko3d-test/openrac-rac1-pal-deko3d.nro, 29268365 bytes.
- SHA256: d3336e08d49f9186b92a51d64e160202d86a8ca28d57ed6c6b8870be4cfad8a3.
- ELF: build/deko3d-test/last-build.elf, 164960120 bytes.
- ELF SHA256: 7e84626736aa68fe680a133cc61d054d6c04234f03f5920ec3540271adbdefd0.
- Build ID: 07eff264285ecd8c66b27ae851d582e1d972f466.
- Both copies hash-identical to compiler outputs.
- All eighteen patched DKSH hashes match provenance and staged RomFS files.
- All eighteen stock DKSH files also match staged comparison-pack files.
- ELF contains patched-pack selection, texture-cache and Zcull logging markers.
- Updated last-build.json (previous manifest described an older build).

Previous artifacts and build log are preserved in
build/deko3d-test/diagnostics/2026-10-29-before-latest-claude-build/.
Its saved old manifest may be stale; use actual artifact hashes if identifying it.

## Next hardware run

Default settings first. Use build/deko3d-test/env-next-test.txt as a guide, keep
comparison switches commented and remove OPENRAC_FRAME_COST for meaningful FPS.
Check title/model, landing ship, near-camera Novalis ground, vendor preview and
HUD. Collect session log, startup log, new driver-log content and clips. Confirm
patched-pack, texture-cache and sampler-rebind startup markers. Do not toggle all
fallbacks at once; reserve those for a controlled comparison after the default run.
