// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Shared arithmetic for offline scene, material and transport code.
#pragma once
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

#include "enhanced/lighting/bake.h"

namespace openrac::lighting::bake_detail {
// Circle constant shared by transport and deterministic direction generation.
inline constexpr float kPi = std::numbers::pi_v<float>;

// Scalar product without fused arithmetic.
inline float dot(Float3 a, Float3 b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// Add positions or radiance components.
inline Float3 add(Float3 a, Float3 b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

// Scale a vector without changing accumulation order.
inline Float3 mul(Float3 a, float s) {
    return {a[0] * s, a[1] * s, a[2] * s};
}

// Geometric cross product for two-sided surface normals.
inline Float3 cross(Float3 a, Float3 b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

// Normalise a finite nonzero vector, rejecting invalid scene data.
inline Float3 unit(Float3 a) {
    const auto length = std::sqrt(dot(a, a));

    // Invalid directions must fail before division or Embree traversal.
    if (!std::isfinite(length) || length <= 0) {
        throw std::runtime_error("invalid bake vector");
    }

    return mul(a, 1 / length);
}

// Check all three components before building acceleration.
inline bool finite(Float3 v) {
    return std::all_of(v.begin(), v.end(), [](float x) { return std::isfinite(x); });
}

}  // namespace openrac::lighting::bake_detail
