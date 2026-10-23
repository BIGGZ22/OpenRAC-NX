// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Compare portable batched integration with the original scalar directional filter.
#include <algorithm>
#include <bit>
#include <cfenv>
#include <cmath>
#include <exception>
#include <numbers>
#include <vector>

#include "enhanced/lighting/bake_integration.h"
#include "tests/check.h"

using namespace openrac::lighting;
using namespace openrac::lighting::bake_detail;

namespace {
// Inputs include capped misses, close blockers and optional explicit emission.
struct Input {
    std::vector<RaySample> rays;
    std::vector<EmissionSample> emission;
    float exponent = 32;
    float cap = 100;
};

// Asymmetric rays exercise distinct per-texel kernels rather than only a constant environment.
Input fixture() {
    Input input;
    constexpr unsigned kRays = 129;
    for (unsigned i = 0; i < kRays; ++i) {
        const float z = 1 - 2 * (static_cast<float>(i) + .5f) / kRays;
        const float r = std::sqrt(std::max(0.0f, 1 - z * z));
        const float angle = static_cast<float>(i) * 2.39996323f;
        input.rays.push_back(
            {{r * std::cos(angle), r * std::sin(angle), z},
             {static_cast<float>(i % 7), .25f, static_cast<float>(i % 3) * .1f},
             i % 5 == 0 ? input.cap : .25f * static_cast<float>(i)}
        );
    }
    input.emission = {{{0, 0, 1}, {1, 2, .5f}}, {{0, -1, 0}, {.1f, .2f, .3f}}};
    return input;
}

// Zero-initialised inactive lanes reproduce the final partial block of an odd map resolution.
DirectionBlock footprint(unsigned active) {
    DirectionBlock block;
    for (unsigned lane = 0; lane < active; ++lane) {
        for (unsigned sample = 0; sample < kIntegrationDirections; ++sample) {
            const unsigned sample_row = sample / 2;
            const unsigned texel_row = lane / 3;
            const float u = (static_cast<float>(sample % 2) + .5f) / 2;
            const float v = (static_cast<float>(sample_row) + .5f) / 2;
            const auto direction = oct_decode(
                {(static_cast<float>(lane % 3) + u) / 3, (static_cast<float>(texel_row) + v) / 3}
            );
            block.x[sample][lane] = direction[0];
            block.y[sample][lane] = direction[1];
            block.z[sample][lane] = direction[2];
        }
    }
    return block;
}

// Characterisation of the pre-vectorisation scalar kernel, retaining general pow for every exponent.
IntegratedSample reference(const DirectionBlock& block, std::size_t lane, const Input& input) {
    IntegratedSample result;
    double weight = 0, first = 0, second = 0;
    for (const auto& ray : input.rays) {
        float cosine = 0;
        double w = 0;
        for (std::size_t sample = 0; sample < kIntegrationDirections; ++sample) {
            const float c = std::max(
                0.0f,
                block.x[sample][lane] * ray.direction[0] + block.y[sample][lane] * ray.direction[1]
                    + block.z[sample][lane] * ray.direction[2]
            );
            cosine += c / 4;
            w += std::pow(static_cast<double>(c), input.exponent) / 4;
        }
        for (unsigned c = 0; c < 3; ++c) {
            result.irradiance[c] += ray.radiance[c] * cosine;
        }
        weight += w;
        first += w * ray.distance;
        second += w * ray.distance * ray.distance;
    }
    for (auto& channel : result.irradiance) {
        channel *= 4 * std::numbers::pi_v<float> / static_cast<float>(input.rays.size());
    }
    for (const auto& light : input.emission) {
        float cosine = 0;
        for (std::size_t sample = 0; sample < kIntegrationDirections; ++sample) {
            cosine += std::max(
                          0.0f,
                          block.x[sample][lane] * light.direction[0]
                              + block.y[sample][lane] * light.direction[1]
                              + block.z[sample][lane] * light.direction[2]
                      )
                      / 4;
        }
        for (unsigned c = 0; c < 3; ++c) {
            result.irradiance[c] +=
                light.radiance_weight[c] * cosine / static_cast<float>(input.emission.size());
        }
    }
    result.moments = {input.cap, input.cap * input.cap};
    if (weight > 1e-20) {
        result.moments = {static_cast<float>(first / weight), static_cast<float>(second / weight)};
    }
    return result;
}

// Squaring may differ from libm by one final float ULP; other exponents must remain bit-identical.
void check_sample(
    const IntegratedSample& actual, const IntegratedSample& expected, float exponent
) {
    CHECK(actual.irradiance == expected.irradiance);
    for (unsigned c = 0; c < 2; ++c) {
        const auto a = std::bit_cast<std::uint32_t>(actual.moments[c]);
        const auto b = std::bit_cast<std::uint32_t>(expected.moments[c]);
        const auto ulps = a > b ? a - b : b - a;
        const unsigned tolerance = exponent == 32 ? 1 : 0;
        CHECK(ulps <= tolerance);
        CHECK(std::isfinite(actual.moments[c]));
    }
}

// Every lane is checked independently, including inactive footprints and their capped fallback.
void check_block(const DirectionBlock& block, const Input& input) {
    const auto actual =
        integrate_probe_block(block, input.rays, input.emission, input.exponent, input.cap);
    for (std::size_t lane = 0; lane < kIntegrationLanes; ++lane) {
        check_sample(actual[lane], reference(block, lane, input), input.exponent);
    }
}

// The exact exponent dispatch must retain general powers, empty emission and single-ray inputs.
void test_kernels() {
    auto input = fixture();
    for (const float exponent : {1.0f, 7.5f, 32.0f, 32.25f, 256.0f}) {
        input.exponent = exponent;
        for (const unsigned active : {0u, 1u, 3u, 4u}) {
            check_block(footprint(active), input);
        }
    }
    input.emission.clear();
    input.exponent = 32;
    check_block(footprint(4), input);
    input.rays.resize(1);
    check_block(footprint(3), input);
}

// The kernel must preserve the caller's arithmetic mode throughout both power paths.
void test_rounding_mode(int mode) {
    CHECK(std::fesetround(mode) == 0);
    test_kernels();
    CHECK(std::fegetround() == mode);
}
}  // namespace

int main() {
    const int rounding = std::fegetround();
    try {
        for (const int mode : {FE_TONEAREST, FE_TOWARDZERO}) {
            test_rounding_mode(mode);
        }
    } catch (const std::exception& error) {
        std::fesetround(rounding);
        std::fprintf(stderr, "integration fixture failed: %s\n", error.what());
        return 1;
    }
    CHECK(std::fesetround(rounding) == 0);
    return openrac::test::result();
}
