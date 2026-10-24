// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The sun's shadow cascades: how the camera's view is split into four cascades, each cascade's
// light-space matrix snapped to its texels, the volume its casters can lie in, and the key the
// static shadow cache (shadow-static-cache) compares. Pure arithmetic: the level renderer draws
// the cascades (level_renderer.cpp, prepare_sun_shadow) and the shadow atlas holds them
// (shadow_atlas.h). Restructure package P2-39a moved it from enhanced/viewer/.

#pragma once
#include <cstdint>
#include <cstring>

#include "renderer/renderer.h"

namespace openrac::scene {
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

// What a sun cascade's static casters (terrain, ties, shrubs) in the static shadow cache were
// drawn for (shadow-static-cache, docs/port/RENDER_PERF.md, R-18): the cascade's snapped
// view-projection, its caster volume (the receiver footprint, which follows the camera), whether
// the hero's casters were left to the hero map, and the generation of the scene's static layers.
// Equal keys draw the same static depth, so a hit copies the cached rectangle instead.
struct StaticShadowKey {
    bool valid = false;                            // False until a cascade is drawn.
    std::array<float, 16> matrix{};                // The cascade's view-projection.
    std::array<std::array<float, 4>, 6> planes{};  // Its caster volume's planes.
    unsigned plane_count = 0;                      // How many of them are used.
    bool separate_hero = false;                    // The hero's casters are the hero map's.
    std::uint64_t generation = 0;                  // LevelScene's static layers' generation.
};

// Returns whether `a` and `b` are the same key. The floats are compared by their bits, so that a
// matrix that differs only in -0 against +0, or in a NaN, misses (never a stale hit).
inline bool same_static_key(const StaticShadowKey& a, const StaticShadowKey& b) {
    return a.valid && b.valid && a.plane_count == b.plane_count
           && a.separate_hero == b.separate_hero && a.generation == b.generation
           && std::memcmp(a.matrix.data(), b.matrix.data(), sizeof(a.matrix)) == 0
           && std::memcmp(a.planes.data(), b.planes.data(), sizeof(a.planes)) == 0;
}
}  // namespace openrac::scene
