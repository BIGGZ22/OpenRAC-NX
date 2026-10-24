// SPDX-License-Identifier: GPL-3.0-or-later
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
