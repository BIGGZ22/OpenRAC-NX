// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The settings of the ambient occlusion pass (enhanced/renderer/gtao.h): the frame carries them
// (FrameInput::gtao_settings), the player's settings keep them (game/common/settings.h). The
// renderer owns the type so that neither it nor the settings include enhanced/; restructure package
// P2-07 moved it from enhanced/renderer/.

#pragma once

namespace openrac::renderer {

// How carefully the horizons are refined: three silhouette-refinement steps, or seven.
enum class GtaoQuality {
    Performance,
    Quality
};

// The ambient occlusion's settings, with the defaults the player's settings start from
// (docs/port/SETTINGS.md, the `gtao_*` keys).
struct GtaoSettings {
    GtaoQuality quality = GtaoQuality::Performance;
    int slices = 4;               // Horizon-search directions.
    int steps = 6;                // samples on each side of a slice
    bool half_resolution = true;  // Evaluated at half size and upsampled along edges.
    float radius = 1.25f;         // Occlusion radius, in scene units.
    float strength = 1.0f;        // multiplier of occlusion; zero leaves the picture unchanged
};

}  // namespace openrac::renderer
