// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once

#include <optional>
#include <span>
#include <vector>

#include "renderer/math.h"

namespace openrac::renderer {

// A posed retail shadow proxy, in world units. Nonpositive segment counts
// describe flat capsule ends; negative counts extend the end by -count/256.
struct ShadowCapsule {
    Vec3 a{}, b{};
    float radius_a = 0, radius_b = 0;
    int segments_a = 0, segments_b = 0;
};

// Closed, consistently wound prisms of the proxies' silhouettes perpendicular
// to extrusion. Caps make depth-fail stencil counting work inside a volume too.
std::vector<Vec3> shadow_volumes(
    std::span<const ShadowCapsule> capsules,
    Vec3 extrusion,
    std::optional<std::array<float, 2>> heights = std::nullopt
);

}  // namespace openrac::renderer
