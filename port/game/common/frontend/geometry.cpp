// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's geometry (geometry.h): the frame box, the frame size and the blends between ticks.

#include "game/common/frontend/geometry.h"

#include <algorithm>
#include <cmath>

namespace openrac::frontend {

Box frame_box(int window_width, int window_height, float aspect) {
    int width = window_width;
    int height = aspect == kTvAspect ? width * 3 / 4 : static_cast<int>(std::lround(static_cast<float>(width) / aspect));
    if (height > window_height) {
        height = window_height;
        width = aspect == kTvAspect
                    ? height * 4 / 3
                    : std::min(window_width, static_cast<int>(std::lround(static_cast<float>(height) * aspect)));
    }
    return {(window_width - width) / 2, (window_height - height) / 2, width, height};
}

void frame_size(int box_width, int box_height, int render_scale, int& width, int& height) {
    constexpr int kLargest = 8192;
    const float scale = static_cast<float>(std::clamp(render_scale, 25, 400)) / 100.0f;
    const float fit = std::min(1.0f, static_cast<float>(kLargest)
                                         / (scale * static_cast<float>(std::max({box_width, box_height, 1}))));
    width = std::max(1, static_cast<int>(std::lround(static_cast<float>(box_width) * scale * fit)));
    height = std::max(1, static_cast<int>(std::lround(static_cast<float>(box_height) * scale * fit)));
}

bool same_moby(const renderer::Mat4& from, const renderer::Mat4& to) {
    const float dx = to[12] - from[12], dy = to[13] - from[13], dz = to[14] - from[14];
    if (dx * dx + dy * dy + dz * dz > 16.0f) {
        return false;
    }
    float dot = 0.0f, la = 0.0f, lb = 0.0f;
    for (int i = 0; i < 3; ++i) {
        dot += from[i] * to[i];
        la += from[i] * from[i];
        lb += to[i] * to[i];
    }
    return la > 0.0f && lb > 0.0f && dot >= 0.7f * std::sqrt(la * lb);
}

renderer::Mat4 blend_matrix(const renderer::Mat4& from, const renderer::Mat4& to, float t) {
    const float dx = to[12] - from[12], dy = to[13] - from[13], dz = to[14] - from[14];
    if (dx * dx + dy * dy + dz * dz > 16.0f) {
        return to;  // a moby slot used again, a teleport
    }
    // Where it is always moves with the blend: a thing carried by another (the wrench in the
    // hero's hand) turns fast, more in a tick than the same-moby test allows, and one left at this
    // tick's place while its carrier is drawn part way there runs ahead of the hand. Only its
    // turning is taken whole when it turned too far to blend its axes.
    renderer::Mat4 m = to;
    const int upto = same_moby(from, to) ? 12 : 0;
    for (int i = 0; i < upto; ++i) {
        m[i] = from[i] + (to[i] - from[i]) * t;
    }
    for (int i = 12; i < 15; ++i) {
        m[i] = from[i] + (to[i] - from[i]) * t;
    }
    return m;
}

CameraPose blend_pose(const CameraPose& from, const CameraPose& to, float t) {
    CameraPose c = to;
    const renderer::Vec3& p0 = from.position;
    const renderer::Vec3& p1 = to.position;
    const float d2 = (p1[0] - p0[0]) * (p1[0] - p0[0]) + (p1[1] - p0[1]) * (p1[1] - p0[1])
                     + (p1[2] - p0[2]) * (p1[2] - p0[2]);
    const float turn = from.forward[0] * to.forward[0] + from.forward[1] * to.forward[1]
                       + from.forward[2] * to.forward[2];
    // A cut: the camera moved or turned too far in a tick to be the same shot moving.
    if (d2 > 64.0f || turn < 0.7f) {
        return c;
    }
    for (int i = 0; i < 3; ++i) {
        c.position[i] = p0[i] + (p1[i] - p0[i]) * t;
    }
    renderer::Vec3 f, u;
    for (int i = 0; i < 3; ++i) {
        f[i] = from.forward[i] + (to.forward[i] - from.forward[i]) * t;
        u[i] = from.up[i] + (to.up[i] - from.up[i]) * t;
    }
    c.forward = renderer::normalize(f);
    c.left = renderer::normalize(renderer::cross(u, c.forward));
    c.up = renderer::cross(c.forward, c.left);
    return c;
}

}  // namespace openrac::frontend
