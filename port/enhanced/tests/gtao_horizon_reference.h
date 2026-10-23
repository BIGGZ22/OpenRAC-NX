// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "renderer/math.h"

namespace openrac::test::gtao_cpu {
using V = std::array<float, 3>;

inline V add(V a, V b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

inline V sub(V a, V b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

inline V mul(V a, float b) {
    return {a[0] * b, a[1] * b, a[2] * b};
}

inline float dot(V a, V b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline V unit(V a) {
    float l = std::sqrt(dot(a, a));
    return l > 0 ? mul(a, 1 / l) : V{};
}

inline V cross(V a, V b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

inline float primitive(float a, float nx, float ny) {
    float s = std::sin(a), c = std::cos(a);
    return .5f * (nx * s * s + ny * (a - s * c));
}

inline float smooth(float a, float b, float x) {
    float t = std::clamp((x - a) / (b - a), 0.f, 1.f);
    return t * t * (3 - 2 * t);
}

struct Scene {
    int w, h;
    renderer::Mat4 p;
    const std::vector<float>& pixels;

    float depth(int x, int y) const {
        return pixels[(std::size_t(std::clamp(y, 0, h - 1)) * w + std::clamp(x, 0, w - 1)) * 4];
    }

    V position(int x, int y, float d) const {
        float z = p[14] / (d * 2.f - 1.f + p[10]);
        return {
            (((float(x) + .5f) / float(w) * 2.f - 1.f + p[8]) / p[0]) * z,
            (((float(y) + .5f) / float(h) * 2.f - 1.f + p[9]) / p[5]) * z,
            -z
        };
    }

    V delta(int x, int y, int dx, int dy, V c) const {
        if (x + dx < 0 || y + dy < 0 || x + dx >= w || y + dy >= h) {
            return {0, 0, 1e6f};
        }
        float d = depth(x + dx, y + dy);
        return d > 0 ? sub(position(x + dx, y + dy, d), c) : V{0, 0, 1e6f};
    }

    V normal(int x, int y, V c) const {
        V l = delta(x, y, -1, 0, c), r = delta(x, y, 1, 0, c), d = delta(x, y, 0, -1, c),
          u = delta(x, y, 0, 1, c);
        if (std::min(std::abs(l[2]), std::abs(r[2])) > 1e5f
            || std::min(std::abs(d[2]), std::abs(u[2])) > 1e5f) {
            return {};
        }
        V n = cross(
            std::abs(l[2]) < std::abs(r[2]) ? mul(l, -1) : r,
            std::abs(d[2]) < std::abs(u[2]) ? mul(d, -1) : u
        );
        if (dot(n, n) < 1e-12f) {
            return {};
        }
        n = unit(n);
        return dot(n, mul(c, -1)) < 0 ? mul(n, -1) : n;
    }

    bool occludes(int tx, int ty, float d, int x, int y, V c, V n, float radius) const {
        float q = depth(x, y) * 2.f - 1.f + p[10], ratio = q / (d * 2.f - 1.f + p[10]);
        float sx = n[0] * (-2.f * c[2]) / (float(w) * p[0]);
        float sy = n[1] * (-2.f * c[2]) / (float(h) * p[5]);
        float plane = dot(n, c) * (ratio - 1.f) + (sx * float(tx - x) + sy * float(ty - y)) * ratio;
        float mag = std::abs(n[0]) * std::abs(c[0]) + std::abs(n[1]) * std::abs(c[1])
                    + std::abs(n[2]) * std::abs(c[2]);
        float error = 16.f * 1.1920928955078125e-7f * (mag + 96.f * (std::abs(sx) + std::abs(sy)))
                      * (std::abs(ratio) + 1.f);
        if (std::abs(plane - radius * .002f) <= error) {
            plane = dot(n, sub(position(tx, ty, d), c));
        }
        return plane > radius * .002f;
    }

    float evaluate(int cx, int cy, int scale, float radius, bool early, bool open) const {
        int x = cx * scale, y = cy * scale;
        if (scale == 2) {
            float nearest = -1;
            for (int i = 0; i < 4; ++i) {
                int a = cx * 2 + (i & 1), b = cy * 2 + (i >> 1);
                if (a < w && b < h && depth(a, b) > nearest) {
                    nearest = depth(a, b);
                    x = a;
                    y = b;
                }
            }
        }
        float z = depth(x, y);
        if (z <= 0) {
            return 1;
        }
        V c = position(x, y, z), n = normal(x, y, c);
        if (dot(n, n) < .1f) {
            return 1;
        }
        V view = unit(mul(c, -1));
        float pixelRadius =
            std::clamp(radius * std::abs(p[0]) * float(w) / (-2.f * c[2]), 1.f, 96.f);
        std::uint32_t seed = std::uint32_t(cx) * 1973u + std::uint32_t(cy) * 9277u + 89173u;
        seed = (seed ^ (seed >> 16u)) * 2246822519u;
        seed = (seed ^ (seed >> 13u)) * 3266489917u;
        seed ^= seed >> 16u;
        float phaseX = (float(seed & 65535u) + .5f) / 65536.f,
              phaseY = (float(seed >> 16u) + .5f) / 65536.f;
        constexpr float pi = 3.14159265359f;
        float visible = 0, total = 0;
        for (int slice = 0; slice < 4; ++slice) {
            float angle = (float(slice) + phaseX) * pi / 4.f, dx = std::cos(angle),
                  dy = std::sin(angle);
            V tangent = {dx / (float(w) * p[0]), dy / (float(h) * p[5]), 0};
            tangent = unit(sub(tangent, mul(view, dot(tangent, view))));
            float nx = dot(n, view), ny = dot(n, tangent), nl = std::sqrt(nx * nx + ny * ny);
            if (nl < 1e-5f) {
                continue;
            }
            nx /= nl;
            ny /= nl;
            float na = std::atan2(ny, nx);
            float initial[2] = {ny, -ny}, horizon[2] = {ny, -ny};
            bool found[2] = {false, false};
            float previous = 0;
            for (int step = 1; step <= 8; ++step) {
                float t = step == 8 ? 1.f : (float(step - 4) + phaseY) / 4.f;
                float distance = step <= 3
                                     ? float(step)
                                     : 3.f * (1 - t * t) + std::max(3.f, pixelRadius) * (t * t);
                for (int side = 0; side < 2; ++side) {
                    float sign = side == 0 ? -1.f : 1.f;
                    int tx = int(std::round(float(x) + dx * distance * sign)),
                        ty = int(std::round(float(y) + dy * distance * sign));
                    if (tx < 0 || ty < 0 || tx >= w || ty >= h) {
                        continue;
                    }
                    float td = depth(tx, ty);
                    if (td <= 0) {
                        continue;
                    }
                    V value = sub(position(tx, ty, td), c);
                    if (early ? !occludes(tx, ty, td, x, y, c, n, radius)
                              : dot(n, value) <= radius * .002f) {
                        continue;
                    }
                    if (step > 3 && !found[side]) {
                        float lower = previous, upper = distance;
                        for (int refine = 0; refine < 7; ++refine) {
                            float middle = (lower + upper) * .5f;
                            int a = int(std::round(float(x) + dx * middle * sign)),
                                b = int(std::round(float(y) + dy * middle * sign));
                            float d = depth(a, b);
                            V candidate = sub(position(a, b, std::max(d, 1e-8f)), c);
                            bool hit =
                                d > 0
                                && (early ? occludes(a, b, std::max(d, 1e-8f), x, y, c, n, radius)
                                          : dot(n, candidate) > radius * .002f);
                            if (hit) {
                                upper = middle;
                                value = candidate;
                            } else {
                                lower = middle;
                            }
                        }
                    }
                    found[side] = true;
                    float ds = dot(value, value);
                    if (early && (ds < 1e-10f || ds >= radius * radius)) {
                        continue;
                    }
                    float dw = std::sqrt(ds);
                    if (dw < 1e-5f || dw >= radius) {
                        continue;
                    }
                    float weight = 1 - smooth(radius * .65f, radius, dw),
                          candidate = dot(view, mul(value, 1 / dw));
                    horizon[side] =
                        std::max(horizon[side], initial[side] * (1 - weight) + candidate * weight);
                }
                previous = distance;
            }
            float full = (nx + na * ny) * nl, integral = full;
            if (!open || horizon[0] > initial[0] || horizon[1] > initial[1]) {
                float lower =
                    std::max(-std::acos(std::clamp(horizon[0], -1.f, 1.f)), na - pi * .5f);
                float upper = std::min(std::acos(std::clamp(horizon[1], -1.f, 1.f)), na + pi * .5f);
                integral =
                    (upper > lower ? primitive(lower, nx, ny) + primitive(upper, nx, ny)
                                         - 2 * primitive(std::clamp(0.f, lower, upper), nx, ny)
                                   : 0)
                    * nl;
            }
            visible += integral;
            total += full;
        }
        return std::clamp(visible / std::max(total, 1e-5f), 0.f, 1.f);
    }
};
}  // namespace openrac::test::gtao_cpu
