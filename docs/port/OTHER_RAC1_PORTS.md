# Other Ratchet & Clank 1 projects: what OpenRAC-NX can use (checked 2026-10-29)

Local shallow clones were in the Claude container at `/home/claude/rr/` (not on the PC).

| Project | Revision | Kind | Language / renderer | Platforms | Licence |
|---|---|---|---|---|---|
| [rac1-native](https://github.com/RAC-Native/rac1-native-game) | b4c85ff 2026-10-28 | function-by-function C++ port + generated code | C++, OpenGL / Vulkan | Linux, Windows (ARM64/Android branch in progress) | MIT |
| [rac-rs](https://github.com/AsobiCodes/rac-rs) | 5a93984 2026-10-28 | static recompilation | Rust, Vulkan / D3D12 | Windows, Linux | GPL-3.0 |
| [ReRAC](https://github.com/re-rac/rerac) | 46c1948 2026-10-28 | rewrite on Bevy, not yet playable through | Rust | desktop | ISC |
| [OpenRatchet](https://github.com/Nikityyy/OpenRatchet) | 07c78d3 2026-09-09 | PS2Recomp + native replacement, early | C++ | Windows (MSVC) | — |
| [Lombyte](https://github.com/lombyte-project/Lombyte) | be4134e 2026-10-29 | matching decompilation (NTSC SCUS_971.99) | C (PS2) | PS2 | GPL-3.0 |

All licences are compatible with OpenRAC's GPL-3.0 for borrowing, with attribution. None targets the
Switch or deko3d, so nothing addresses the dkgl translation bugs directly.

## Useful for OpenRAC-NX, in order

1. **The vendor screen and menu item models (if still wrong after the Zcull and feedback build).**
   - rac1-native draws "the Vendor's monitors, the pause menu's item models" into per-panel render
     targets: `src/game/render/mrp_2d.cpp` (comment at line 216) and `render_view.cpp:181`. The
     latter uses the game's own field-of-view scale on the Vendor's screens (game mode 5).
   - ReRAC's `docs/test-requests/2026-10-01-space-visuals.md` documents `VendorStartWeaponDemo` /
     `VendorModeUpdate` and the weapon-demo actor.
   - Compare with OpenRAC's `renderer/direct.cpp` offscreen squares and
     `frontend/scene_hud.cpp` `draw_target_mobys`.
2. **The landing / space-flight scene (level 98).** ReRAC's `space-visuals.md` covers:
   - the flight's ship from the `spaceships` file, evaluated from a streamed animation chunk (no
     skeleton: a stand-in);
   - the canopy glass, the ship's shadow depth fade, and the take-off and landing actors;
   - music handling: silent during a Kerwan landing, back after.
   This is a reference for what the scene should show and when its audio starts.
3. **GPU cost of alpha-tested batches.**
   - rac1-native tests the second pass (the failing pixels, no depth write) strictly (LESS instead
     of LEQUAL), so pixels the first pass drew are not shaded again: 10–15 % of world GPU time.
   - It is not exact for coplanar faces, so it is an option there (`RAC1_MRPW_P2_LESS`).
   - OpenRAC's cutout passes (`translucent_pass`) are analogous. With reversed depth, use GREATER
     instead of GEQUAL.
   - Only worth it once the Switch is GPU-bound. It is CPU-bound now (submit 8–13 ms).
4. **Frame capture only when read.**
   - rac1-native copies the last frame for the pause menu only when it is read: 0.5–0.6 ms GPU saved.
   - OpenRAC copies the world into `last_world` every world frame (`scene_hud.cpp`
     `retain_background` → `before_hud` → `blit_to`). On dkgl that is a copy-engine copy plus two
     Full barriers per frame.
   - A lazy version needs care: the menu needs the world without the HUD.
5. **Threading.** rac1-native pipelines the frame (game update on its own thread, drawing one frame
   behind). OpenRAC's threaded presentation is Vulkan-only (`frontend/interpolation.cpp:464`). On
   Switch the game update is under 1 ms, so the gain would be small.

## Not useful for the Switch
- rac-rs's GPU skinning and culling: a different renderer; dkgl's per-draw cost is the bottleneck.
- rac1-native's AVX2 VU0 SIMD: x86 only. ARM NEON could follow the same idea, but the update is not
  the bottleneck.
- Enhanced lighting, GTAO and cascades: off on Switch.

See also `docs/port/SWITCH_DEKO3D_PORTS_NOTES.md`: Switch ports (not RaC1) that moved from OpenGL or
Vulkan to deko3d, and the fixes taken from them.
