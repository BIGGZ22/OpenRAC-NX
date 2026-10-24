// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once
#include <cstdint>
#include <span>
#include <vector>

#include "renderer/shadow_volume.h"

namespace openrac::scene {
struct MobyShadowState {
    bool enabled = false;
    renderer::Vec3 extrusion{};
    std::array<float, 2> heights{};
    float scale = 0;
    std::uint8_t direction_index = 0;
};

// MobyProc (NTSC 0x212738..0x2128bc): selected direction, ground slab,
// mode gates and distance fade. The game updates the table and moby fields.
MobyShadowState moby_shadow_state(
    std::span<const std::uint8_t> ram,
    std::uint32_t moby,
    std::uint32_t directions,
    const renderer::Mat4& view
);

// The relocated class block, posed with P_j (not the mesh's P_j S_j).
std::vector<renderer::ShadowCapsule> moby_shadows(
    std::span<const std::uint8_t> ram,
    std::uint32_t moby,
    const renderer::Mat4& model,
    float scale = 1.0f
);
}  // namespace openrac::scene
