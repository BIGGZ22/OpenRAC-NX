// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The sun's shadow cascades of sun_shadow.h: the split, the snapped matrices, the caster volumes.

#include "scene/sun_shadow.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace openrac::scene {
namespace {
using namespace renderer;

Mat4 inverse(const Mat4& m) {
    double a[4][8]{};
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            a[r][c] = m[c * 4 + r];
        }
        a[r][r + 4] = 1;
    }
    for (int c = 0; c < 4; ++c) {
        int pivot = c;
        for (int r = c + 1; r < 4; ++r) {
            if (std::abs(a[r][c]) > std::abs(a[pivot][c])) {
                pivot = r;
            }
        }
        if (!std::isfinite(a[pivot][c]) || std::abs(a[pivot][c]) < 1e-12) {
            throw std::runtime_error("singular shadow camera");
        }
        for (int k = 0; k < 8; ++k) {
            std::swap(a[c][k], a[pivot][k]);
        }
        const double scale = a[c][c];
        for (double& value : a[c]) {
            value /= scale;
        }
        for (int r = 0; r < 4; ++r) {
            if (r == c) {
                continue;
            }
            const double factor = a[r][c];
            for (int k = 0; k < 8; ++k) {
                a[r][k] -= factor * a[c][k];
            }
        }
    }
    Mat4 result;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            result[c * 4 + r] = static_cast<float>(a[r][c + 4]);
        }
    }
    return result;
}

Vec3 unproject(const Mat4& m, float x, float y, float z) {
    auto p = transform_point(m, {x, y, z});
    float w = m[3] * x + m[7] * y + m[11] * z + m[15];
    if (!std::isfinite(w) || std::abs(w) < 1e-12f) {
        throw std::runtime_error("invalid shadow projection");
    }
    for (auto& v : p) {
        v /= w;
    }
    return p;
}

SunCascades::CasterVolume caster_volume(
    const std::array<Vec3, 8>& corners, const Mat4& basis, float margin
) {
    using Point = std::array<float, 2>;
    std::array<Point, 8> points;
    for (std::size_t i = 0; i < corners.size(); ++i) {
        const auto p = transform_point(basis, corners[i]);
        if (!std::isfinite(p[0]) || !std::isfinite(p[1])) {
            return {};
        }
        points[i] = {p[0], p[1]};
    }
    std::sort(points.begin(), points.end());
    const auto turn = [](Point a, Point b, Point c) {
        return double(b[0] - a[0]) * double(c[1] - a[1])
               - double(b[1] - a[1]) * double(c[0] - a[0]);
    };
    std::array<Point, 16> hull;
    unsigned count = 0;
    for (const auto p : points) {
        while (count >= 2 && turn(hull[count - 2], hull[count - 1], p) <= 0) {
            --count;
        }
        hull[count++] = p;
    }
    const unsigned lower = count;
    for (int i = 6; i >= 0; --i) {
        const auto p = points[static_cast<std::size_t>(i)];
        while (count > lower && turn(hull[count - 2], hull[count - 1], p) <= 0) {
            --count;
        }
        hull[count++] = p;
    }
    if (count) {
        --count;  // repeated first point
    }
    SunCascades::CasterVolume result;
    if (count < 3 || count > result.planes.size()) {
        return result;
    }
    for (unsigned i = 0; i < count; ++i) {
        const Point a = hull[i], b = hull[(i + 1) % count];
        const float x = a[1] - b[1], y = b[0] - a[0];
        const float length = std::hypot(x, y);
        if (!(length > 1e-6f)) {
            return {};
        }
        auto& plane = result.planes[i];
        for (unsigned k = 0; k < 3; ++k) {
            plane[k] = (x * basis[k * 4] + y * basis[k * 4 + 1]) / length;
        }
        plane[3] = -(x * a[0] + y * a[1]) / length + margin;
    }
    result.plane_count = count;
    return result;
}
}  // namespace

SunCascades sun_cascades(
    const renderer::Camera& camera,
    renderer::Vec3 direction,
    renderer::Vec3 low,
    renderer::Vec3 high,
    float distance
) {
    using namespace renderer;
    if (!std::isfinite(distance) || distance <= 1 || !std::isfinite(dot(direction, direction))
        || dot(direction, direction) < 1e-8f) {
        throw std::runtime_error("invalid sun shadow configuration");
    }
    const auto inv_projection = inverse(camera.projection), inv_view = inverse(camera.view);
    SunCascades result;
    result.near_distance =
        std::clamp(-unproject(inv_projection, 0, 0, 1)[2], 0.05f, distance * 0.1f);
    const auto basis = look_along(
        {0, 0, 0},
        direction,
        std::abs(normalize(direction)[2]) > .95f ? Vec3{0, 1, 0} : Vec3{0, 0, 1}
    );
    float previous = result.near_distance;
    for (unsigned i = 0; i < kSunCascades; ++i) {
        const float fraction = static_cast<float>(i + 1) / kSunCascades;
        result.ends[i] =
            kSunSplitLambda * result.near_distance
                * std::pow(distance / result.near_distance, fraction)
            + (1 - kSunSplitLambda)
                  * (result.near_distance + (distance - result.near_distance) * fraction);
        // Include the preceding cascade's transition region in this fit.
        const float start =
            i ? previous - 0.1f * (previous - (i > 1 ? result.ends[i - 2] : result.near_distance))
              : previous;
        std::array<Vec3, 8> corners;
        Vec3 centre{};
        for (int c = 0; c < 8; ++c) {
            const float x = (c & 1) ? 1.f : -1.f, y = (c & 2) ? 1.f : -1.f;
            const auto a = unproject(inv_projection, x, y, -.5f),
                       b = unproject(inv_projection, x, y, .5f);
            const float z = -((c & 4) ? result.ends[i] : start);
            if (std::abs(b[2] - a[2]) < 1e-8f) {
                throw std::runtime_error("invalid shadow camera depth");
            }
            Vec3 p;
            for (int k = 0; k < 3; ++k) {
                p[k] = a[k] + (b[k] - a[k]) * (z - a[2]) / (b[2] - a[2]);
            }
            corners[c] = transform_point(inv_view, p);
            for (int k = 0; k < 3; ++k) {
                centre[k] += corners[c][k] / 8;
            }
        }
        float radius = 0;
        for (auto p : corners) {
            for (int k = 0; k < 3; ++k) {
                p[k] -= centre[k];
            }
            radius = std::max(radius, std::sqrt(dot(p, p)));
        }
        radius = std::ceil(radius * 16) / 16;
        const int resolution = kSunResolutions[i];
        radius *=
            static_cast<float>(resolution) / (resolution - 6);  // snapping + filtered PCF margin
        auto light_centre = transform_point(basis, centre);
        const float texel = 2 * radius / resolution;
        // PCF reaches at most two texel centres from the receiver. A third
        // texel plus 0.05 world units covers snapping, normal bias and roundoff.
        result.casters[i] = caster_volume(corners, basis, 3 * texel + 0.05f);
        for (int k = 0; k < 2; ++k) {
            light_centre[k] = std::round(light_centre[k] / texel) * texel;
        }
        float zmin = light_centre[2] - radius - 32, zmax = light_centre[2] + radius + 32;
        if (low[0] <= high[0]) {
            for (int c = 0; c < 8; ++c) {
                Vec3 p;
                for (int k = 0; k < 3; ++k) {
                    p[k] = (c & (1 << k)) ? high[k] : low[k];
                }
                const float z = transform_point(basis, p)[2];
                zmin = std::min(zmin, z - 32);
                zmax = std::max(zmax, z + 32);
            }
        }
        Mat4 ortho = identity();
        ortho[0] = ortho[5] = 1 / radius;
        ortho[12] = -light_centre[0] / radius;
        ortho[13] = -light_centre[1] / radius;
        ortho[10] = 2 / (zmax - zmin);
        ortho[14] = -(zmax + zmin) / (zmax - zmin);
        result.matrices[i] = multiply(ortho, basis);
        previous = result.ends[i];
    }
    return result;
}
}  // namespace openrac::scene
