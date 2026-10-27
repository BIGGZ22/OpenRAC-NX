// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The little matrix arithmetic the renderer and the viewer need: column-major
// 4 x 4 matrices (GL's order, and glTF's), in the game's axes (Z up).

#pragma once

#include <array>
#include <cmath>

namespace openrac::renderer {

using Vec3 = std::array<float, 3>;
using Mat4 = std::array<float, 16>;  // column-major: element (row r, column c) at c * 4 + r

constexpr Mat4 identity() {
    return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

constexpr Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 out{};
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a[k * 4 + r] * b[c * 4 + k];
            }
            out[c * 4 + r] = sum;
        }
    }
    return out;
}

constexpr Vec3 transform_point(const Mat4& m, const Vec3& p) {
    return {
        m[0] * p[0] + m[4] * p[1] + m[8] * p[2] + m[12],
        m[1] * p[0] + m[5] * p[1] + m[9] * p[2] + m[13],
        m[2] * p[0] + m[6] * p[1] + m[10] * p[2] + m[14]
    };
}

constexpr Mat4 scale(float s) {
    return {s, 0, 0, 0, 0, s, 0, 0, 0, 0, s, 0, 0, 0, 0, 1};
}

constexpr Mat4 translation(const Vec3& t) {
    return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, t[0], t[1], t[2], 1};
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

inline float dot(const Vec3& a, const Vec3& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline Vec3 normalize(const Vec3& v) {
    const float length = std::sqrt(dot(v, v));
    return length > 0.0f ? Vec3{v[0] / length, v[1] / length, v[2] / length} : v;
}

// A camera at `eye` looking along `forward`, with `up` roughly up: world to
// view space (GL's: X right, Y up, looking down -Z).
inline Mat4 look_along(const Vec3& eye, const Vec3& forward, const Vec3& up) {
    const Vec3 f = normalize(forward);
    const Vec3 s = normalize(cross(f, up));
    const Vec3 u = cross(s, f);
    return {
        s[0],
        u[0],
        -f[0],
        0,
        s[1],
        u[1],
        -f[1],
        0,
        s[2],
        u[2],
        -f[2],
        0,
        -dot(s, eye),
        -dot(u, eye),
        dot(f, eye),
        1
    };
}

// A perspective projection with reversed depth, the convention the game's
// depth test has (RENDERER.md section 5, OpenGOAL's renderer): the near plane
// lands at depth 1 and the far plane at 0, the depth buffer is cleared to 0
// and the test is GEQUAL. `fov_y` in radians.
inline Mat4 perspective_reversed(float fov_y, float aspect, float near_z, float far_z) {
    const float f = 1.0f / std::tan(fov_y / 2.0f);
    // GL's usual projection, with its Z row negated.
    return {
        f / aspect,
        0,
        0,
        0,
        0,
        f,
        0,
        0,
        0,
        0,
        -(far_z + near_z) / (near_z - far_z),
        -1,
        0,
        0,
        -(2.0f * far_z * near_z) / (near_z - far_z),
        0
    };
}

// perspective_reversed with a frustum that need not be centred (a headset's eye, OpenXR's
// XrFovf): the angles of its left, right, top and bottom edges from straight ahead, in radians
// (left and down negative).
inline Mat4 perspective_off_centre_reversed(
    float angle_left, float angle_right, float angle_up, float angle_down, float near_z, float far_z
) {
    const float left = std::tan(angle_left);
    const float right = std::tan(angle_right);
    const float up = std::tan(angle_up);
    const float down = std::tan(angle_down);
    return {
        2.0f / (right - left),
        0,
        0,
        0,
        0,
        2.0f / (up - down),
        0,
        0,
        (right + left) / (right - left),
        (up + down) / (up - down),
        -(far_z + near_z) / (near_z - far_z),
        -1,
        0,
        0,
        -(2.0f * far_z * near_z) / (near_z - far_z),
        0
    };
}

// The view of an eye at `position`, turned by the unit quaternion `orientation` (x, y, z, w), both
// in the view space of `view` (GL's, as OpenXR's: X right, Y up, looking down -Z): world to the
// eye's view space.
inline Mat4 eye_view(
    const Mat4& view, const Vec3& position, const std::array<float, 4>& orientation
) {
    const float x = orientation[0], y = orientation[1], z = orientation[2], w = orientation[3];
    // The eye's rotation, transposed (its inverse), column-major.
    const Mat4 inverse_rotation = {
        1 - 2 * (y * y + z * z),
        2 * (x * y - z * w),
        2 * (x * z + y * w),
        0,
        2 * (x * y + z * w),
        1 - 2 * (x * x + z * z),
        2 * (y * z - x * w),
        0,
        2 * (x * z - y * w),
        2 * (y * z + x * w),
        1 - 2 * (x * x + y * y),
        0,
        0,
        0,
        0,
        1,
    };
    return multiply(
        inverse_rotation, multiply(translation({-position[0], -position[1], -position[2]}), view)
    );
}

}  // namespace openrac::renderer
