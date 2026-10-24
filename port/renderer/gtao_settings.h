// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The settings of the ambient occlusion pass (enhanced/renderer/gtao.h): the frame carries them
// (FrameInput::gtao_settings), the player's settings keep them (game/common/settings.h). The
// renderer owns the type so that neither it nor the settings include enhanced/; restructure package
// P2-07 moved it from enhanced/renderer/.

#pragma once

namespace openrac::renderer {
enum class GtaoQuality { Performance, Quality };

struct GtaoSettings {
    GtaoQuality quality = GtaoQuality::Performance;
    int slices = 4;
    int steps = 6;  // samples on each side of a slice
    bool half_resolution = true;
    float radius = 1.25f;
    float strength = 1.0f;  // multiplier of occlusion; zero leaves the picture unchanged
};
}  // namespace openrac::renderer
