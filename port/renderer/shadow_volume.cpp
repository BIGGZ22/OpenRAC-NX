// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#include "renderer/shadow_volume.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace openrac::renderer {
namespace {
Vec3 add(Vec3 a, Vec3 b, float scale = 1) {
    for (int i = 0; i < 3; ++i) {
        a[i] += scale * b[i];
    }
    return a;
}

bool finite(Vec3 v) {
    return std::all_of(v.begin(), v.end(), [](float x) { return std::isfinite(x); });
}

using Point = std::array<float, 2>;

float turn(Point a, Point b, Point c) {
    return (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
}

std::vector<Point> hull(std::vector<Point> p) {
    std::sort(p.begin(), p.end());
    p.erase(std::unique(p.begin(), p.end()), p.end());
    if (p.size() < 3) {
        return {};
    }
    std::vector<Point> out;
    for (const auto& v : p) {
        while (out.size() >= 2 && turn(out[out.size() - 2], out.back(), v) <= 0) {
            out.pop_back();
        }
        out.push_back(v);
    }
    const auto lower = out.size();
    for (std::size_t i = p.size() - 1; i-- > 0;) {
        while (out.size() > lower && turn(out[out.size() - 2], out.back(), p[i]) <= 0) {
            out.pop_back();
        }
        out.push_back(p[i]);
    }
    out.pop_back();
    return out;
}
}  // namespace

std::vector<Vec3> shadow_volumes(
    std::span<const ShadowCapsule> capsules,
    Vec3 extrusion,
    std::optional<std::array<float, 2>> heights
) {
    std::vector<Vec3> triangles;
    if (!finite(extrusion) || dot(extrusion, extrusion) < 1.0e-8f) {
        return triangles;
    }
    if (heights
        && (!std::isfinite((*heights)[0]) || !std::isfinite((*heights)[1])
            || (*heights)[0] >= (*heights)[1] || extrusion[2] >= -1e-5f)) {
        return triangles;
    }
    const Vec3 d = normalize(extrusion);
    const Vec3 u = normalize(cross(std::fabs(d[2]) < 0.9f ? Vec3{0, 0, 1} : Vec3{1, 0, 0}, d));
    const Vec3 v = cross(d, u);
    const auto tri = [&](Vec3 a, Vec3 b, Vec3 c) {
        triangles.insert(triangles.end(), {a, b, c});
    };
    for (const auto& c : capsules) {
        if (!finite(c.a) || !finite(c.b) || !std::isfinite(c.radius_a) || !std::isfinite(c.radius_b)
            || c.radius_a < 0 || c.radius_b < 0 || std::max(c.radius_a, c.radius_b) <= 0) {
            continue;
        }
        Vec3 a = c.a, b = c.b;
        const Vec3 delta = add(b, a, -1);
        if (c.segments_a < 0) {
            a = add(a, delta, static_cast<float>(c.segments_a) / 256.0f);
        }
        if (c.segments_b < 0) {
            b = add(b, delta, -static_cast<float>(c.segments_b) / 256.0f);
        }
        Vec3 side = normalize(cross(d, delta));
        if (dot(side, side) < 0.5f) {
            side = u;
        }
        std::vector<Point> points;
        const auto end = [&](Vec3 p, float radius, int segments) {
            const int n = segments > 0 ? std::clamp(segments, 3, 64) : 2;
            for (int i = 0; i < n; ++i) {
                Vec3 q;
                if (segments <= 0) {
                    q = add(p, side, i == 0 ? radius : -radius);
                } else {
                    const float angle = 2 * std::numbers::pi_v<float>
                                        * static_cast<float>(i) / static_cast<float>(n);
                    q = add(add(p, u, radius * std::cos(angle)), v, radius * std::sin(angle));
                }
                points.push_back({dot(q, u), dot(q, v)});
            }
        };
        end(a, c.radius_a, c.segments_a);
        end(b, c.radius_b, c.segments_b);
        const auto outline = hull(std::move(points));
        if (outline.size() < 3) {
            continue;
        }
        const float start = std::min(dot(a, d) - c.radius_a, dot(b, d) - c.radius_b);
        std::vector<Vec3> near, far;
        for (const auto& p : outline) {
            const Vec3 q = add(add(Vec3{}, u, p[0]), v, p[1]);
            const auto origin = add(q, d, start);
            if (heights) {
                // Retail 0x228520 intersects both ends of every extrusion ray
                // with the moby's upper and lower receiver planes.
                near.push_back(add(origin, d, ((*heights)[1] - origin[2]) / d[2]));
                far.push_back(add(origin, d, ((*heights)[0] - origin[2]) / d[2]));
            } else {
                near.push_back(origin);
                far.push_back(add(origin, extrusion));
            }
        }
        for (std::size_t i = 1; i + 1 < near.size(); ++i) {
            tri(near[0], near[i + 1], near[i]);
            tri(far[0], far[i], far[i + 1]);
        }
        for (std::size_t i = 0; i < near.size(); ++i) {
            const auto j = (i + 1) % near.size();
            tri(near[i], near[j], far[j]);
            tri(near[i], far[j], far[i]);
        }
    }
    return triangles;
}
}  // namespace openrac::renderer
