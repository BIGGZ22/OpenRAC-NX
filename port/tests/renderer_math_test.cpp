// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The view math a mod's views use (renderer/math.h, game/common/frontend/views.cpp): an eye moved
// and turned from the game's camera, and a frustum off its centre, against the centred projection
// and the camera they reduce to.

#include <array>
#include <cmath>

#include "check.h"
#include "renderer/math.h"

namespace {

using openrac::renderer::eye_view;
using openrac::renderer::look_along;
using openrac::renderer::Mat4;
using openrac::renderer::multiply;
using openrac::renderer::perspective_off_centre_reversed;
using openrac::renderer::perspective_reversed;
using openrac::renderer::transform_point;
using openrac::renderer::Vec3;

bool near(float a, float b) {
    return std::fabs(a - b) < 1e-4f;
}

bool near(const Mat4& a, const Mat4& b) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!near(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

bool near(const Vec3& a, const Vec3& b) {
    return near(a[0], b[0]) && near(a[1], b[1]) && near(a[2], b[2]);
}

const Mat4 kCamera = look_along({10, 20, 5}, {0, 1, 0}, {0, 0, 1});  // the game's axes: Z up

// The identity pose is the camera itself.
void identity_is_the_camera() {
    CHECK(near(eye_view(kCamera, {0, 0, 0}, {0, 0, 0, 1}), kCamera));
}

// An eye moved right by 0.5 sees the camera's own position 0.5 to its left.
void a_moved_eye() {
    const Mat4 eye = eye_view(kCamera, {0.5f, 0, 0}, {0, 0, 0, 1});
    CHECK(near(transform_point(eye, {10, 20, 5}), {-0.5f, 0, 0}));
}

// An eye turned 90 degrees left (about view Y) sees what was to the camera's left straight ahead.
void a_turned_eye() {
    const float h = std::sqrt(0.5f);
    const Mat4 eye = eye_view(kCamera, {0, 0, 0}, {0, h, 0, h});
    // The camera looks along +Y with Z up, so its left is -X.
    CHECK(near(transform_point(eye, {10 - 3, 20, 5}), {0, 0, -3}));
}

// A frustum with equal angles is the centred projection; an off-centre one moves its middle.
void an_off_centre_frustum() {
    const float half = 0.6f;
    const float aspect = std::tan(0.8f) / std::tan(half);
    CHECK(near(
        perspective_off_centre_reversed(-0.8f, 0.8f, half, -half, 0.05f, 2000.0f),
        perspective_reversed(2 * half, aspect, 0.05f, 2000.0f)
    ));
    const Mat4 p = perspective_off_centre_reversed(-0.9f, 0.7f, 0.6f, -0.6f, 0.05f, 2000.0f);
    // A point straight ahead lands right of the middle (the frustum reaches further left), by
    // -(r + l) / (r - l) of the half-width.
    const Mat4 projected = multiply(p, Mat4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -1, 1});
    const float l = std::tan(-0.9f), r = std::tan(0.7f);
    CHECK(projected[12] / projected[15] > 0.0f);
    CHECK(near(projected[12] / projected[15], -(r + l) / (r - l)));
}

}  // namespace

int main() {
    identity_is_the_camera();
    a_moved_eye();
    a_turned_eye();
    an_off_centre_frustum();
    return openrac::test::result();
}
