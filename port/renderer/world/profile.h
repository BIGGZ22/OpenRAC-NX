// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the world renderers need to know about one game version: the values its
// code sets (WorldConstants) and the shape of its frame. Everything else in
// renderer/world is the same for every game; a new title or region is added
// here, not in the renderers.
//
// Known: RAC1 NTSC-U (ReRAC's reading) and RAC1 PAL (frame and view from
// docs/engine/systems/CAMERA_FOG.md and HARDWARE.md; its constants are taken
// to be NTSC-U's, not yet checked in the PAL executable). Rac2 to Rac4: not
// known yet, world_profile() returns nothing.

#pragma once

#include <optional>

#include "assets/version.h"
#include "renderer/world/draw_data.h"

namespace openrac::renderer::world {

struct WorldProfile {
    WorldConstants constants;
    // The game's draw buffer, in pixels: particle and star sprites, the HUD
    // and the overlays are sized in it.
    float frame_width = 512.0f;
    float frame_height = 448.0f;
    // tan_y / tan_x of the game's own view.
    float frame_y_ratio = 0.756f;
};

std::optional<WorldProfile> world_profile(assets::Game game, assets::Region region);
std::optional<WorldProfile> world_profile(const assets::GameVersion& version);

}  // namespace openrac::renderer::world
