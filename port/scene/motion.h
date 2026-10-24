// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Displayed-frame motion inputs, independent of any temporal reconstruction implementation.
#pragma once

#include <array>
#include <cstdint>

#include "renderer/math.h"

namespace openrac::scene {
// A rigid or skinned object at the current and previous displayed frame.
struct MotionInstance {
    std::uint32_t model = 0;              // Scene model index.
    renderer::Mat4 current{};             // Current world transform.
    renderer::Mat4 previous{};            // Last displayed world transform.
    std::array<int, 2> palettes{-1, -1};  // Current and previous first joint matrix.
    float alpha = 1;                      // Authored instance opacity.
    bool reactive = false;                // New objects reject stale history.
    bool exclude_glow = false;            // Authored glow vertices never own opaque depth.
};

// Camera matrices excluding jitter, plus the jittered matrix used for rasterisation.
struct MotionView {
    renderer::Mat4 raster{};        // Current jittered world-to-clip transform.
    renderer::Mat4 current{};       // Current unjittered world-to-clip transform.
    renderer::Mat4 previous{};      // Previous unjittered world-to-clip transform.
    std::array<int, 2> size{};      // Motion units are current render pixels.
    unsigned previous_palette = 0;  // Borrowed previous-frame joint texture.
};
}  // namespace openrac::scene
