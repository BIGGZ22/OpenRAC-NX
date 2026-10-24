#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Static world and sky motion from depth, excluding the raster jitter from both endpoints.
uniform sampler2D tex_T0;
uniform mat4 inverse_raster;
uniform mat4 previous_view;
uniform vec3 camera_position;
uniform vec2 jitter;
uniform int depth_only;
layout(location=0) out vec4 result;
void main() {
    vec2 size = vec2(textureSize(tex_T0, 0));
    vec2 uv = gl_FragCoord.xy / size;
    float depth = texelFetch(tex_T0, ivec2(gl_FragCoord.xy), 0).r;
    if (depth_only != 0) {
        result = vec4(depth);
        return;
    }
    vec4 world = inverse_raster * vec4(uv * 2 - 1, depth * 2 - 1, 1);
    world /= world.w;
    // A sky pixel follows camera rotation, without the translation of a finite far-plane surface.
    if (depth <= 0) {
        world = vec4(world.xyz - camera_position, 0);
    }
    vec4 old = previous_view * world;
    bool valid = old.w > 0;
    vec2 previous_uv = valid ? old.xy / old.w * 0.5 + 0.5 : uv;
    vec2 current_uv = uv - jitter / size;
    result = vec4((previous_uv - current_uv) * size, valid ? 0 : 1, 0);
}
