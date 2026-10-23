// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Portable offline integration kernel; no Embree or platform-specific vector types.
#pragma once

#include <array>
#include <span>

#include "enhanced/lighting/bake.h"

namespace openrac::lighting::bake_detail {

// Independent texels occupy lanes; each lane retains the original ray and subtexel order.
inline constexpr std::size_t kIntegrationLanes = 4;
inline constexpr std::size_t kIntegrationDirections =
    std::size_t{kProbeTexelSamplesPerAxis} * kProbeTexelSamplesPerAxis;

// Structure-of-arrays footprint lets ordinary C++ loops vectorise across texels.
struct DirectionBlock {
    using Component = std::array<std::array<float, kIntegrationLanes>, kIntegrationDirections>;
    Component x{}, y{}, z{};
};

// One accepted primary hit, including its unbounded transport and capped visibility distance.
struct RaySample {
    Float3 direction, radiance;
    float distance;
};

// Explicit emitter estimate shared by the same texel footprints as traced radiance.
struct EmissionSample {
    Float3 direction{}, radiance_weight{};
};

// Linear irradiance and normalised distance moments for one texel.
struct IntegratedSample {
    Float3 irradiance{};
    Float2 moments{};
};

// Requires nonempty rays and validated bake settings. Unused tail lanes may have zero directions.
// Exponent 32 uses repeated double-precision squaring; other exponents retain general pow.
[[nodiscard]] std::array<IntegratedSample, kIntegrationLanes> integrate_probe_block(
    const DirectionBlock& directions,
    std::span<const RaySample> rays,
    std::span<const EmissionSample> emission,
    float distance_exponent,
    float max_distance
);

}  // namespace openrac::lighting::bake_detail
