// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Batch independent texels in standard C++ without changing per-texel reduction order.
#include "enhanced/lighting/bake_integration.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace openrac::lighting::bake_detail {
namespace {
// Contiguous lanes allow SIMD without a platform-specific vector API.
template <typename T>
using Lanes = std::array<T, kIntegrationLanes>;

// Radiance and distance kernels use the same subtexel dot products.
struct RayWeights {
    Lanes<float> cosine{};
    Lanes<double> distance{};
};

// Separate arrays avoid interleaving RGB and double-precision moment accumulators.
struct Accumulator {
    Lanes<float> red{}, green{}, blue{};
    Lanes<double> weight{}, first{}, second{};
};

// Each iteration updates an independent texel, never a cross-lane sum.
template <bool Power32>
RayWeights ray_weights(const DirectionBlock& block, Float3 direction, float exponent) {
    RayWeights result;
    constexpr float kSampleWeight = 1 / static_cast<float>(kIntegrationDirections);
    for (std::size_t sample = 0; sample < kIntegrationDirections; ++sample) {
        for (std::size_t lane = 0; lane < kIntegrationLanes; ++lane) {
            const float cosine = std::max(
                0.0f,
                block.x[sample][lane] * direction[0] + block.y[sample][lane] * direction[1]
                    + block.z[sample][lane] * direction[2]
            );
            result.cosine[lane] += cosine * kSampleWeight;
            double weight;
            // Exact exponent dispatch leaves other configured exponents on their original kernel.
            if constexpr (Power32) {
                weight = static_cast<double>(cosine);
                weight *= weight;
                weight *= weight;
                weight *= weight;
                weight *= weight;
                weight *= weight;
            } else {
                weight = std::pow(static_cast<double>(cosine), exponent);
            }
            result.distance[lane] += weight * kSampleWeight;
        }
    }
    return result;
}

// Explicit emission needs cosine weights only, without distance-power calculations.
Lanes<float> emission_weights(const DirectionBlock& block, Float3 direction) {
    Lanes<float> result{};
    constexpr float kSampleWeight = 1 / static_cast<float>(kIntegrationDirections);
    for (std::size_t sample = 0; sample < kIntegrationDirections; ++sample) {
        for (std::size_t lane = 0; lane < kIntegrationLanes; ++lane) {
            const float cosine = std::max(
                0.0f,
                block.x[sample][lane] * direction[0] + block.y[sample][lane] * direction[1]
                    + block.z[sample][lane] * direction[2]
            );
            result[lane] += cosine * kSampleWeight;
        }
    }
    return result;
}

// Rays remain the outer reduction: the compiler can pack independent texel accumulations.
template <bool Power32>
Accumulator accumulate_rays(
    const DirectionBlock& directions, std::span<const RaySample> rays, float exponent
) {
    Accumulator result;
    for (const auto& ray : rays) {
        const auto weights = ray_weights<Power32>(directions, ray.direction, exponent);
        for (std::size_t lane = 0; lane < kIntegrationLanes; ++lane) {
            result.red[lane] += ray.radiance[0] * weights.cosine[lane];
            result.green[lane] += ray.radiance[1] * weights.cosine[lane];
            result.blue[lane] += ray.radiance[2] * weights.cosine[lane];
            result.weight[lane] += weights.distance[lane];
            result.first[lane] += weights.distance[lane] * ray.distance;
            result.second[lane] += weights.distance[lane] * ray.distance * ray.distance;
        }
    }
    return result;
}

// Apply the original diffuse solid-angle factor and cap fallback independently in each lane.
std::array<IntegratedSample, kIntegrationLanes> finish_rays(
    const Accumulator& sums, std::size_t ray_count, float max_distance
) {
    std::array<IntegratedSample, kIntegrationLanes> result{};
    const float scale = 4 * std::numbers::pi_v<float> / static_cast<float>(ray_count);
    for (std::size_t lane = 0; lane < kIntegrationLanes; ++lane) {
        result[lane].irradiance =
            {sums.red[lane] * scale, sums.green[lane] * scale, sums.blue[lane] * scale};
        result[lane].moments = {max_distance, max_distance * max_distance};
        // Unsupported footprints retain capped distances rather than introducing zero-distance walls.
        if (sums.weight[lane] > 1e-20) {
            result[lane].moments =
                {static_cast<float>(sums.first[lane] / sums.weight[lane]),
                 static_cast<float>(sums.second[lane] / sums.weight[lane])};
        }
    }
    return result;
}
}  // namespace

std::array<IntegratedSample, kIntegrationLanes> integrate_probe_block(
    const DirectionBlock& directions,
    std::span<const RaySample> rays,
    std::span<const EmissionSample> emission,
    float distance_exponent,
    float max_distance
) {
    // Select once per block; the specialised ray loop contains no exponent branch or pow calls.
    const auto sums = distance_exponent == 32
                          ? accumulate_rays<true>(directions, rays, distance_exponent)
                          : accumulate_rays<false>(directions, rays, distance_exponent);
    auto result = finish_rays(sums, rays.size(), max_distance);
    for (const auto& light : emission) {
        const auto weights = emission_weights(directions, light.direction);
        for (std::size_t lane = 0; lane < kIntegrationLanes; ++lane) {
            for (unsigned channel = 0; channel < 3; ++channel) {
                result[lane].irradiance[channel] += light.radiance_weight[channel] * weights[lane]
                                                    / static_cast<float>(emission.size());
            }
        }
    }
    return result;
}

}  // namespace openrac::lighting::bake_detail
