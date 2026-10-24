// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Deterministic two-sided transport and occupancy for offline probes.
#pragma once
#include <limits>
#include <span>

#include "enhanced/lighting/bake_integration.h"
#include "enhanced/lighting/bake_scene.h"

namespace openrac::lighting::bake_detail {
// Per-probe sequence; preserve consumption order across transport changes.
struct Random {
    std::uint32_t state;  // Mutable sequence owned by a single probe worker.

    // Advance the original 32-bit LCG and return a 24-bit fraction.
    float next() {
        state = state * 1664525u + 1013904223u;
        return static_cast<float>(state >> 8) / 16777216.0f;
    }
};

// Paired crossing parity and the nearest exit candidate.
struct Placement {
    bool inside = false;  // Paired crossings vote for a closed interior.
    float nearest = std::numeric_limits<float>::infinity();  // Nearest accepted crossing.
    Float3 exit_direction{};                                 // Ray toward that crossing.
};

// Sample a visible emitter without depending on authored shading normals.
EmissionSample sample_emitter(const Scene& scene, Float3 origin, float bias, Random& random);
// Classify a probe with paired rays so winding cannot decide occupancy.
Placement classify(
    const Scene& scene,
    Float3 position,
    const BakeSettings& settings,
    std::span<const std::array<Float3, 2>> directions
);
// Continue from the shared primary hit; distance moments keep that hit separately.
Float3 radiance(const Scene& scene, const BakeSettings& settings, RTCRayHit& hit, Random& random);
}  // namespace openrac::lighting::bake_detail
