// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <cstring>

namespace openrac::viewer {
inline constexpr int kShadowAtlasSize = 3072;
// x, y, width, height; four sun cascades followed by the hero view.
inline constexpr std::array<std::array<int, 4>, 5> kShadowAtlasRects{
    {{0, 0, 2048, 2048},
     {2048, 0, 1024, 1024},
     {2048, 1024, 1024, 1024},
     {0, 2048, 1024, 1024},
     {1024, 2048, 512, 512}}
};

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
}  // namespace openrac::viewer
