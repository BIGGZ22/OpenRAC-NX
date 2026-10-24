#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Particle colour changes and new opaque objects reduce temporal history confidence.
uniform sampler2D tex_T0;
uniform sampler2D tex_T1;
uniform sampler2D tex_T2;
layout(location=0) out vec4 result;
void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 difference = abs(texelFetch(tex_T0, pixel, 0).rgb - texelFetch(tex_T1, pixel, 0).rgb);
    // Conservative coverage for alpha-blended and additive effects without opaque depth.
    float reactive = clamp(4 * max(difference.r, max(difference.g, difference.b)), 0, 1);
    reactive = max(reactive, texelFetch(tex_T2, pixel, 0).z);
    result = vec4(reactive, 0, 0, 1);
}
