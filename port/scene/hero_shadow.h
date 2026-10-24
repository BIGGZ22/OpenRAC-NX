// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Ratchet's shadow map: whether the hero is in the air (read from game memory), and the small
// orthographic view along the main light that his shadow is drawn from. Pure arithmetic; the
// level renderer draws the map (level_renderer.cpp, prepare_hero_shadow). Restructure package
// P2-39a moved it from enhanced/viewer/.

#pragma once
#include <cstdint>
#include <optional>
#include <span>

#include "renderer/math.h"

namespace openrac::scene {
struct HeroShadowProjection {
    renderer::Mat4 matrix = renderer::identity();
    renderer::Vec3 direction{0, 0, -1};  // light travel direction
};

// The collision/ground-probe update maintains air_frames at Hero+0x30E,
// including the zero-vertical-speed jump apex. 0x30A is not jump state.
std::optional<bool> hero_airborne(std::span<const std::uint8_t> ram, std::uint32_t hero);
HeroShadowProjection hero_shadow_projection(
    renderer::Vec3 centre, renderer::Vec3 main_direction, bool airborne
);
}  // namespace openrac::scene
