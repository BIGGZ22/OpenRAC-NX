// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
uniform sampler2D tex_T0;
uniform mat4 projection;
uniform int evaluation_scale;
const float pi = 3.14159265359;

float depth_at(ivec2 p) {
    return texelFetch(tex_T0, clamp(p, ivec2(0), textureSize(tex_T0, 0) - 1), 0).r;
}

vec3 position_at(ivec2 pixel, float depth) {
    vec2 ndc = (vec2(pixel) + 0.5) / vec2(textureSize(tex_T0, 0)) * 2.0 - 1.0;
    float distance = projection[3][2] / (depth * 2.0 - 1.0 + projection[2][2]);
    return vec3(
               (ndc + vec2(projection[2][0], projection[2][1]))
                   / vec2(projection[0][0], projection[1][1]),
               -1.0
           )
           * distance;
}

ivec2 source_pixel(ivec2 cell, out int selected) {
    if (evaluation_scale == 1) {
        selected = 0;
        return cell;
    }
    ivec2 extent = textureSize(tex_T0, 0);
    ivec2 result = min(cell * 2, extent - 1);
    float nearest = -1.0;
    selected = 0;
    for (int i = 0; i < 4; ++i) {
        ivec2 p = cell * 2 + ivec2(i & 1, i >> 1);
        if (any(greaterThanEqual(p, extent))) {
            continue;
        }
        float d = depth_at(p);
        if (d > nearest) {
            nearest = d;
            result = p;
            selected = i;
        }
    }
    return result;
}

vec2 encode_normal(vec3 n) {
    n /= abs(n.x) + abs(n.y) + abs(n.z);
    vec2 folded = (1.0 - abs(n.yx)) * mix(vec2(-1), vec2(1), greaterThanEqual(n.xy, vec2(0)));
    return (n.z < 0.0 ? folded : n.xy) * 0.5 + 0.5;
}

vec3 decode_normal(vec2 encoded) {
    vec2 xy = encoded * 2.0 - 1.0;
    vec3 n = vec3(xy, 1.0 - abs(xy.x) - abs(xy.y));
    float t = clamp(-n.z, 0.0, 1.0);
    n.xy += mix(vec2(t), vec2(-t), greaterThanEqual(n.xy, vec2(0)));
    return normalize(n);
}
