#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
uniform samplerBuffer tex_T1;
uniform float strength;
layout(location = 0) out vec4 result;

void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    float depth = depth_at(pixel);
    if (depth <= 0.0) {
        result = vec4(1);
        return;
    }
    vec3 centre = position_at(pixel, depth);
    vec3 normal = normal_at(pixel, centre);
    if (dot(normal, normal) < 0.1) {
        result = vec4(1);
        return;
    }
    float visibility = 0.0, total = 0.0;
    ivec2 extent = (textureSize(tex_T0, 0) + evaluation_scale - 1) / evaluation_scale;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            ivec2 cell = clamp(pixel / evaluation_scale + ivec2(x, y), ivec2(0), extent - 1);
            vec4 guide = texelFetch(tex_T1, cell.y * extent.x + cell.x);
            int selected = int(round(guide.g * 3.0));
            ivec2 tap = cell * evaluation_scale
                        + (evaluation_scale == 2 ? ivec2(selected & 1, selected >> 1) : ivec2(0));
            float d = depth_at(tap);
            if (d <= 0.0) {
                continue;
            }
            vec3 delta = position_at(tap, d) - centre;
            // Plane distance preserves sloping surfaces but rejects silhouettes.
            float plane = abs(dot(normal, delta));
            float agreement = max(dot(normal, decode_normal(guide.ba)), 0.0);
            // Hard rejection is needed as well as weights: normalising a
            // tiny mismatched weight alone can transfer foreground AO to a
            // background silhouette, especially in half-resolution mode.
            if (agreement < 0.85 || plane > 0.025) {
                continue;
            }
            if (abs(delta.z) > max(0.05, -centre.z * 0.01) && plane > 0.0025) {
                continue;
            }
            float weight =
                exp(-float(x * x + y * y) * 0.6 - plane * plane / 0.0025) * pow(agreement, 8.0);
            visibility += guide.r * weight;
            total += weight;
        }
    }
    float ao = total > 1e-5 ? visibility / total : 1.0;
    result = vec4(vec3(clamp(1.0 - (1.0 - ao) * strength, 0.0, 1.0)), 1);
}
