// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's geometry (geometry.h): the frame box, the frame size, the blends between ticks
// and the bound of a moby drawn between them.

#include "game/common/frontend/geometry.h"

#include <algorithm>
#include <array>
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

bool usable_camera(const CameraPose& pose) {
    const auto finite = [](const renderer::Vec3& v) {
        return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]);
    };

    // A NaN or an infinity anywhere makes every vertex drawn from it one too.
    if (!finite(pose.position) || !finite(pose.forward) || !finite(pose.up)) {
        return false;
    }

    // An eye out past every level: what a float of garbage reads as.
    for (const float c : pose.position) {
        if (std::fabs(c) > kFarthestEye) {
            return false;
        }
    }

    const float forward = std::sqrt(renderer::dot(pose.forward, pose.forward));
    const float up = std::sqrt(renderer::dot(pose.up, pose.up));

    // The game's rows are unit vectors; one far from it is not a camera's.
    if (!(forward >= 0.5f && forward <= 2.0f && up >= 0.5f && up <= 2.0f)) {
        return false;
    }

    // Forward and up along one line leave no side axis: the view squashes the world flat.
    return std::fabs(renderer::dot(pose.forward, pose.up)) <= 0.999f * forward * up;
}

bool usable_view(const renderer::Mat4& view, const renderer::Mat4& projection) {
    const auto finite = [](const renderer::Mat4& m) {
        return std::all_of(m.begin(), m.end(), [](float x) { return std::isfinite(x); });
    };

    // A NaN or an infinity in either matrix reaches every vertex.
    if (!finite(view) || !finite(projection)) {
        return false;
    }

    // The view's rotation, column-major: entry (row r, column c) is view[4 * c + r].
    const auto at = [&view](int r, int c) { return static_cast<double>(view[4 * c + r]); };
    const double det = at(0, 0) * (at(1, 1) * at(2, 2) - at(1, 2) * at(2, 1))
                       - at(0, 1) * (at(1, 0) * at(2, 2) - at(1, 2) * at(2, 0))
                       + at(0, 2) * (at(1, 0) * at(2, 1) - at(1, 1) * at(2, 0));

    // A rotation keeps volumes: a determinant far from 1 squashes or mirrors the world.
    if (!(det > 0.5 && det < 2.0)) {
        return false;
    }

    return projection[0] > 0.0f && projection[5] > 0.0f;
}

std::array<float, 4> moby_sphere_envelope(
    const std::array<float, 4>& from_sphere,
    const renderer::Mat4& from,
    const std::array<float, 4>& to_sphere,
    const renderer::Mat4& to,
    const renderer::Mat4& drawn
) {
    // Each endpoint sphere bounds its posed vertices in world space. Map it
    // back to a sphere about the model origin: inverse Frobenius norm bounds
    // inverse stretch. The convex blend of the two local poses stays within
    // the larger origin sphere, even while the model itself turns or scales.
    const auto local_radius = [](const std::array<float, 4>& sphere, const renderer::Mat4& matrix) {
        if (!(sphere[3] > 0)
            || !std::all_of(sphere.begin(), sphere.end(), [](float x) { return std::isfinite(x); })
            || !std::all_of(matrix.begin(), matrix.end(), [](float x) { return std::isfinite(x); })
            || matrix[3] != 0 || matrix[7] != 0 || matrix[11] != 0 || matrix[15] != 1) {
            return -1.0;
        }
        const std::array<double, 3> a{matrix[0], matrix[1], matrix[2]};
        const std::array<double, 3> b{matrix[4], matrix[5], matrix[6]};
        const std::array<double, 3> c{matrix[8], matrix[9], matrix[10]};
        const auto cross = [](const auto& x, const auto& y) {
            return std::array<double, 3>{
                x[1] * y[2] - x[2] * y[1], x[2] * y[0] - x[0] * y[2], x[0] * y[1] - x[1] * y[0]
            };
        };
        const auto bc = cross(b, c), ca = cross(c, a), ab = cross(a, b);
        const double determinant = a[0] * bc[0] + a[1] * bc[1] + a[2] * bc[2];
        if (determinant == 0) {
            return -1.0;
        }
        double inverse_squared = 0, offset_squared = 0;
        for (unsigned k = 0; k < 3; ++k) {
            inverse_squared += bc[k] * bc[k] + ca[k] * ca[k] + ab[k] * ab[k];
            const double offset = double(sphere[k]) - matrix[12 + k];
            offset_squared += offset * offset;
        }
        return std::sqrt(inverse_squared) / std::abs(determinant)
               * (std::sqrt(offset_squared) + sphere[3]);
    };
    const double r0 = local_radius(from_sphere, from), r1 = local_radius(to_sphere, to);
    if (r0 < 0 || r1 < 0 || drawn[3] != 0 || drawn[7] != 0 || drawn[11] != 0 || drawn[15] != 1
        || !std::all_of(drawn.begin(), drawn.end(), [](float x) { return std::isfinite(x); })) {
        return {};
    }
    double stretch_squared = 0;
    for (unsigned column = 0; column < 3; ++column) {
        for (unsigned row = 0; row < 3; ++row) {
            stretch_squared += double(drawn[4 * column + row]) * drawn[4 * column + row];
        }
    }
    const float radius = static_cast<float>(std::max(r0, r1) * std::sqrt(stretch_squared));
    if (!(radius > 0) || !std::isfinite(radius)) {
        return {};
    }
    return {drawn[12], drawn[13], drawn[14], radius};
}

}  // namespace openrac::frontend
