// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's geometry (geometry.h): the frame box, the frame size and the blends between ticks.

#include "game/common/frontend/geometry.h"

#include <algorithm>
#include <cmath>

namespace openrac::frontend {
namespace {

// The largest side a texture can have on any card the port runs on, in pixels.
constexpr int kLargestTexture = 8192;

// The render scales the player can choose, in percent of the box.
constexpr int kMinRenderScale = 25;
constexpr int kMaxRenderScale = 400;

// How far a moby may move in a tick and still be the same moby, squared (4 units).
constexpr float kMobyStepSquared = 16.0f;

// How far a camera may move in a tick and still be the same shot, squared (8 units).
constexpr float kCameraStepSquared = 64.0f;

// The least cosine between a tick's axis and the next one's for the same moby or shot (about 45
// degrees).
constexpr float kSameTurnCosine = 0.7f;

}  // namespace

Box frame_box(int window_width, int window_height, float aspect) {
    int width = window_width;
    // The TV's shape in integers, as the game's 4:3 frame divides; any other shape rounded.
    int height = aspect == kTvAspect
                     ? width * 3 / 4
                     : static_cast<int>(std::lround(static_cast<float>(width) / aspect));

    // Too tall for the window at its full width: as tall as the window instead.
    if (height > window_height) {
        height = window_height;
        width = aspect == kTvAspect
                    ? height * 4 / 3
                    : std::min(
                          window_width,
                          static_cast<int>(std::lround(static_cast<float>(height) * aspect))
                      );
    }

    return {(window_width - width) / 2, (window_height - height) / 2, width, height};
}

void frame_size(int box_width, int box_height, int render_scale, int& width, int& height) {
    // A percentage, as a factor.
    const float scale =
        static_cast<float>(std::clamp(render_scale, kMinRenderScale, kMaxRenderScale)) / 100.0f;
    const float fit = std::min(
        1.0f,
        static_cast<float>(kLargestTexture)
            / (scale * static_cast<float>(std::max({box_width, box_height, 1})))
    );
    width = std::max(1, static_cast<int>(std::lround(static_cast<float>(box_width) * scale * fit)));
    height =
        std::max(1, static_cast<int>(std::lround(static_cast<float>(box_height) * scale * fit)));
}

bool same_moby(const renderer::Mat4& from, const renderer::Mat4& to) {
    // Elements 12 to 14 are the translation (column-major, renderer/math.h).
    const float dx = to[12] - from[12];
    const float dy = to[13] - from[13];
    const float dz = to[14] - from[14];

    // Moved too far in a tick: another moby, or a teleport.
    if (dx * dx + dy * dy + dz * dz > kMobyStepSquared) {
        return false;
    }

    float dot = 0.0f;
    float la = 0.0f;
    float lb = 0.0f;

    // The X axes' dot product and squared lengths: elements 0 to 2 are the first column.
    for (int i = 0; i < 3; ++i) {
        dot += from[i] * to[i];
        la += from[i] * from[i];
        lb += to[i] * to[i];
    }

    return la > 0.0f && lb > 0.0f && dot >= kSameTurnCosine * std::sqrt(la * lb);
}

renderer::Mat4 blend_matrix(const renderer::Mat4& from, const renderer::Mat4& to, float t) {
    const float dx = to[12] - from[12];
    const float dy = to[13] - from[13];
    const float dz = to[14] - from[14];

    // Moved too far in a tick to be drawn part way.
    if (dx * dx + dy * dy + dz * dz > kMobyStepSquared) {
        return to;  // a moby slot used again, a teleport
    }

    // Where it is always moves with the blend: a thing carried by another (the wrench in the
    // hero's hand) turns fast, more in a tick than the same-moby test allows, and one left at this
    // tick's place while its carrier is drawn part way there runs ahead of the hand. Only its
    // turning is taken whole when it turned too far to blend its axes.
    renderer::Mat4 m = to;
    // The three axes are elements 0 to 11; none of them when the moby turned too far.
    const int upto = same_moby(from, to) ? 12 : 0;

    // The axes, part way.
    for (int i = 0; i < upto; ++i) {
        m[i] = from[i] + (to[i] - from[i]) * t;
    }

    // The translation, elements 12 to 14, part way.
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
    if (d2 > kCameraStepSquared || turn < kSameTurnCosine) {
        return c;
    }

    // The eye, part way.
    for (int i = 0; i < 3; ++i) {
        c.position[i] = p0[i] + (p1[i] - p0[i]) * t;
    }

    renderer::Vec3 f;
    renderer::Vec3 u;

    // The forward and up axes, part way; normalised and squared up below.
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
