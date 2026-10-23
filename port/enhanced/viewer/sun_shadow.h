// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "renderer/renderer.h"

namespace openrac::viewer {
inline constexpr unsigned kSunCascades = 4;
inline constexpr int kSunResolution = 1024;
inline constexpr std::array<int, kSunCascades> kSunResolutions{2048, 1024, 1024, 1024};
inline constexpr float kSunSplitLambda = 0.85f;

struct SunCascades {
    struct CasterVolume {
        // The receiver footprint extruded infinitely along the sun direction.
        // Empty: a degenerate/unusual projection; retain square-map culling.
        std::array<std::array<float, 4>, 6> planes{};
        unsigned plane_count = 0;
    };

    std::array<renderer::Mat4, kSunCascades> matrices{};
    std::array<CasterVolume, kSunCascades> casters{};
    std::array<float, kSunCascades> ends{};
    float near_distance = 0.1f;
};

// Rigid camera view, arbitrary invertible perspective/orthographic projection.
// Caster bounds extend depth independently of the camera frustum.
SunCascades sun_cascades(
    const renderer::Camera& camera,
    renderer::Vec3 direction,
    renderer::Vec3 caster_low,
    renderer::Vec3 caster_high,
    float distance = 256
);
}  // namespace openrac::viewer
