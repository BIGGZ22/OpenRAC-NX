// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Frozen depth-derived input for the horizon-math equivalence tests only.
// Runtime AO consumes the smooth normal prepass, covered by gtao_vulkan.
vec3 neighbour_delta(ivec2 pixel, ivec2 direction, vec3 centre) {
    ivec2 neighbour = pixel + direction;
    if (any(lessThan(neighbour, ivec2(0)))
        || any(greaterThanEqual(neighbour, textureSize(tex_T0, 0)))) {
        return vec3(0, 0, 1e6);
    }
    float d = depth_at(neighbour);
    return d > 0.0 ? position_at(pixel + direction, d) - centre : vec3(0, 0, 1e6);
}

vec3 normal_at(ivec2 pixel, vec3 centre) {
    vec3 l = neighbour_delta(pixel, ivec2(-1, 0), centre);
    vec3 r = neighbour_delta(pixel, ivec2(1, 0), centre);
    vec3 d = neighbour_delta(pixel, ivec2(0, -1), centre);
    vec3 u = neighbour_delta(pixel, ivec2(0, 1), centre);
    if (min(abs(l.z), abs(r.z)) > 1e5 || min(abs(d.z), abs(u.z)) > 1e5) {
        return vec3(0);
    }
    vec3 n = cross(abs(l.z) < abs(r.z) ? -l : r, abs(d.z) < abs(u.z) ? -d : u);
    if (dot(n, n) < 1e-12) {
        return vec3(0);
    }
    n = normalize(n);
    return dot(n, -centre) < 0.0 ? -n : n;
}
