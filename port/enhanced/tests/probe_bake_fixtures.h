// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Shared analytic geometry and comparisons for offline bake fixtures.
#pragma once
#include <algorithm>
#include <cmath>

#include "enhanced/lighting/bake.h"
#include "tests/check.h"

namespace openrac::lighting::bake_test {
// Build a closed two-sided box with black diffuse surfaces.
inline std::vector<BakeTriangle> box(Float3 low, Float3 high) {
    const Float3 vertices[] =
        {{low[0], low[1], low[2]},
         {high[0], low[1], low[2]},
         {high[0], high[1], low[2]},
         {low[0], high[1], low[2]},
         {low[0], low[1], high[2]},
         {high[0], low[1], high[2]},
         {high[0], high[1], high[2]},
         {low[0], high[1], high[2]}};
    const unsigned faces[][3] =
        {{0, 2, 1},
         {0, 3, 2},
         {4, 5, 6},
         {4, 6, 7},
         {0, 1, 5},
         {0, 5, 4},
         {1, 2, 6},
         {1, 6, 5},
         {2, 3, 7},
         {2, 7, 6},
         {3, 0, 4},
         {3, 4, 7}};
    std::vector<BakeTriangle> triangles;

    for (const auto& face : faces) {
        triangles.push_back({{vertices[face[0]], vertices[face[1]], vertices[face[2]]}, {0, 0, 0}});
    }

    return triangles;
}

// Compare validity, relocation and integrated lighting with fixed tolerances.
inline void equivalent(const ProbeVolume& a, const ProbeVolume& b) {
    CHECK(a.probes.size() == b.probes.size());

    for (std::size_t i = 0; i < a.probes.size(); ++i) {
        CHECK(a.probes[i].valid == b.probes[i].valid);

        for (unsigned c = 0; c < 3; ++c) {
            CHECK(std::abs(a.probes[i].offset[c] - b.probes[i].offset[c]) < 0.0001f);
        }
    }

    for (std::size_t i = 0; i < a.irradiance.size(); ++i) {
        for (unsigned c = 0; c < 3; ++c) {
            CHECK(std::abs(a.irradiance[i][c] - b.irradiance[i][c]) < 0.0001f);
        }

        for (unsigned c = 0; c < 2; ++c) {
            CHECK(std::abs(a.distance_moments[i][c] - b.distance_moments[i][c]) < 0.001f);
        }
    }
}

// Average the upward receiving cosine across a 2x2 probe footprint.
inline float texel_up_cosine(unsigned x, unsigned y, unsigned resolution) {
    float cosine = 0;

    for (const float v : {.25f, .75f}) {
        for (const float u : {.25f, .75f}) {
            const auto direction = oct_decode(
                {(static_cast<float>(x) + u) / static_cast<float>(resolution),
                 (static_cast<float>(y) + v) / static_cast<float>(resolution)}
            );
            cosine += std::max(0.0f, direction[2]) * .25f;
        }
    }

    return cosine;
}

// Reverse source winding while retaining the same occupancy and transport result.
inline void winding_invariant(
    const ProbeGrid& grid, const BakeScene& scene, const BakeSettings& settings
) {
    const auto reference = bake_probes(grid, scene, settings);

    for (unsigned pattern : {0u, 1u, 2u}) {
        auto changed = scene;

        for (std::size_t i = 0; i < changed.triangles.size(); ++i) {
            if (pattern == 2 || i % 2 == pattern) {
                std::swap(changed.triangles[i].vertices[1], changed.triangles[i].vertices[2]);
                std::swap(changed.triangles[i].uv[1], changed.triangles[i].uv[2]);
            }
        }

        equivalent(reference, bake_probes(grid, changed, settings));
    }
}
}  // namespace openrac::lighting::bake_test
