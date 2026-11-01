# Vendor hologram lettering investigation — 2026-10-30

User clarification: GADGETRON spelled-out ring shows only blue lines; reference
image shows cyan lettering orbiting the dark G. Reported present since earliest
builds, so do not attribute to recent PVS/terrain batching without evidence.
Root cause NOT established, visual fix NOT implemented, NO BUILD.

Object: class1143 (0x477), vendor_orb_update in
 games/rac1/src/overlays/shared/vendor_002D9438.c around2936.
It sets shine distance0x20, attaches joint1 manipulator, rotates body and modifier
oppositely. Not class1891 gadgetron_sign_update, which is a different level object.

Existing paths:
- port/scene/moby_pose.cpp reads modifier chain+0x64 and evaluates joint palette.
- port/viewer/shaders/mesh.frag around267 explicitly allows partially transparent
  Gadgetron lettering in glow passes. Check compiled pack counterpart and pass state.
- port/scene/level_renderer.cpp around366 suppresses mipmaps and forces linear
  filtering for cutout moby textures only when image path contains /textures/moby_.
  Verify actual hologram material classification/path before assuming it applies.
- port/scene/moby_passes.h splits glow alpha/depth; inspect interaction with
  fragment discard, cutout, texture alpha, filtering, and material/shine passes.

Codex change ONLY port/game/common/frontend/frame.cpp: optional direct-getenv
OPENRAC_VENDOR_LOGO_TRACE=1 extends existing periodic hero pose diagnostic to
class1143 and logs each primitive's material/image/cutout/metal/index count.
Disabled by default. No renderer behavior changed. New flag not added to env table.
Actual Switch/AArch64 syntax-only check passed with __SWITCH__,frontend,SDL2 and
common/include paths. Initial host/missing-include attempts failed before proper
include configuration; no full game build performed. Existing NRO lacks diagnostic.

Claude: preserve partial work; inspect git diff before edits. Diagnose texture
content/UV/alpha/pass state before touching skeletal animation. Capture actual
broken in-game image if needed; reference screenshot is intended appearance.
Potential mip/path/classification issues are hypotheses, not established bugs.
Do not regenerate patched shader packs with stock UAM. Document a real fix and
its validation; otherwise clearly hand back bounded diagnostics. No build/commit/push.
