// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's geometry (geometry.h): the frame box and the blend of a moby between ticks.

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

}  // namespace openrac::frontend
