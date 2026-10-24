// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Ratchet's shadow projection and airborne flag of hero_shadow.h.

#include "scene/hero_shadow.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace openrac::scene {
namespace {
// Keeps invalid or upward authored lights from forming an unusable ground projection.
renderer::Vec3 ground_shadow_direction(renderer::Vec3 direction) {
    const bool finite =
        std::all_of(direction.begin(), direction.end(), [](float x) { return std::isfinite(x); });
    // Degenerate directions fall back to a downward map, as before interpolation was enabled.
    if (!finite || renderer::dot(direction, direction) <= 1e-8f || direction[2] >= -1e-4f) {
        return {0, 0, -1};
    }
    return renderer::normalize(direction);
}
}  // namespace

float hero_shadow_blend(bool airborne, double seconds, ShadowTransition& transition) {
    // Both directions use the same 200-ms transition after the airborne gate qualifies take-off.
    constexpr double kTransitionSeconds = 0.2;
    constexpr double kResetGapSeconds = 1.0;
    const float target = airborne ? 1.0f : 0.0f;
    const double elapsed = seconds - transition.previous_seconds;

    // Hidden shadows, clock resets and loading gaps must not animate stale history.
    if (transition.previous_seconds < 0 || !std::isfinite(seconds) || elapsed < 0
        || elapsed > kResetGapSeconds) {
        transition.value = target;
        transition.origin = target;
        transition.target = target;
        transition.started_seconds = seconds;
    } else {
        const float progress = static_cast<float>(
            std::clamp((seconds - transition.started_seconds) / kTransitionSeconds, 0.0, 1.0)
        );
        // Cubic ease-out moves quickly initially and settles with zero velocity at either endpoint.
        const float remaining = 1 - progress;
        const float eased = 1 - remaining * remaining * remaining;
        transition.value = transition.origin + (transition.target - transition.origin) * eased;

        // Each direction starts its own ease-out from the currently displayed value.
        if (transition.target != target) {
            transition.origin = transition.value;
            transition.target = target;
            transition.started_seconds = seconds;
        }
    }

    transition.previous_seconds = std::isfinite(seconds) ? seconds : -1.0;
    return transition.value;
}

std::optional<bool> hero_airborne(std::span<const std::uint8_t> ram, std::uint32_t hero) {
    // Collision/ground-probe update clears air_frames on landing and
    // increments it while off the ground, including a zero-speed jump apex.
    // 0x30A instead flags particular mobys hit by the ground probe.
    constexpr std::size_t offset = 0x30e;
    if (!hero || hero > ram.size() || ram.size() - hero < offset + 2) {
        return {};
    }
    std::uint16_t airborne = 0;
    std::memcpy(&airborne, ram.data() + hero + offset, sizeof(airborne));
    // Three consecutive game updates reject brief loss of ground contact before tilting the map.
    constexpr std::uint16_t kMinimumAirFrames = 3;
    return airborne >= kMinimumAirFrames;
}

HeroShadowProjection hero_shadow_projection(
    renderer::Vec3 centre, renderer::Vec3 main_direction, float airborne_blend
) {
    using namespace renderer;
    // Invalid interpolation inputs must not reach the shadow rasteriser as NaN matrices.
    if (!std::isfinite(airborne_blend)) {
        throw std::runtime_error("invalid hero shadow progress");
    }

    for (float n : centre) {
        if (!std::isfinite(n)) {
            throw std::runtime_error("invalid hero shadow centre");
        }
    }
    HeroShadowProjection result;
    result.direction = ground_shadow_direction(main_direction);
    // Keep a fixed horizontal up axis while tilting, avoiding a map-roll change near vertical.
    const Vec3 horizontal_up{-result.direction[1], result.direction[0], 0};
    const Vec3 up =
        dot(horizontal_up, horizontal_up) > 1e-8f ? normalize(horizontal_up) : Vec3{0, 1, 0};
    const float blend = std::clamp(airborne_blend, 0.0f, 1.0f);
    for (float& component : result.direction) {
        component *= 1 - blend;
    }
    result.direction[2] -= blend;
    result.direction = normalize(result.direction);
    constexpr float half_extent = 4, upstream = 4, reach = 72;
    Vec3 eye = centre;
    for (unsigned axis = 0; axis < 3; ++axis) {
        eye[axis] -= result.direction[axis] * upstream;
    }
    const auto view = look_along(eye, result.direction, up);
    // OpenGL reverse Z: near z=0 -> +1, far z=-reach -> -1.
    Mat4 projection = identity();
    projection[0] = projection[5] = 1 / half_extent;
    projection[10] = 2 / reach;
    projection[14] = 1;
    result.matrix = multiply(projection, view);
    return result;
}
}  // namespace openrac::scene
