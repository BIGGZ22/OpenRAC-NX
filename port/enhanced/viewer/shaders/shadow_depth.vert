#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Same instance transform and skin palette as mesh.vert; only depth/alpha
// outputs are needed for both the world cascades and the separate hero map.
layout(location = 0) in vec3 position;
#ifndef OPENRAC_SHADOW_SOLID
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 colour;

layout(location = 8) in vec4 tint;
layout(location = 9) in float palette;
layout(location = 10) in uvec4 joints;
layout(location = 11) in vec4 weights;
layout(location = 12) in float light_slot;
layout(location = 14) in vec4 moby_light;
layout(location = 15) in vec4 moby_ambient;
#endif
layout(location = 4) in mat4 model;
uniform mat4 view_projection;
#ifdef OPENRAC_SHADOW_ATLAS
uniform int shadow_view;
uniform mat4 shadow_view_projection[5];
#endif
invariant gl_Position;
#ifndef OPENRAC_SHADOW_SOLID
uniform int skinning;
uniform sampler2D palette_texture;
out vec2 v_uv;
out vec4 v_colour;
flat out int v_glow;

mat4 joint_matrix(int index) {
    int texel = 4 * index;
    ivec2 at = ivec2(texel % 1024, texel / 1024);
    return mat4(
        texelFetch(palette_texture, at, 0),
        texelFetch(palette_texture, at + ivec2(1, 0), 0),
        texelFetch(palette_texture, at + ivec2(2, 0), 0),
        texelFetch(palette_texture, at + ivec2(3, 0), 0)
    );
}
#endif
void main() {
    vec4 p = vec4(position, 1.0);
#ifndef OPENRAC_SHADOW_SOLID
    float total = weights.x + weights.y + weights.z + weights.w;
    if (skinning != 0 && palette >= 0.0 && total > 0.0) {
        int base = int(palette + 0.5);
        mat4 skin = weights.x * joint_matrix(base + int(joints.x))
                    + weights.y * joint_matrix(base + int(joints.y))
                    + weights.z * joint_matrix(base + int(joints.z))
                    + weights.w * joint_matrix(base + int(joints.w));
        p = vec4((skin * p).xyz, 1.0);
    }
#endif
#ifdef OPENRAC_SHADOW_ATLAS
    gl_Position = shadow_view_projection[shadow_view] * model * p;
    gl_ViewportIndex = shadow_view;
#else
    gl_Position = view_projection * model * p;
#endif
    gl_ClipDistance[0] = 1.0;
    gl_ClipDistance[1] = 1.0;
#ifndef OPENRAC_SHADOW_SOLID
    v_uv = uv;
    v_colour = colour * tint;
    v_glow = abs(moby_light.w) > 0.5 && light_slot < -1.5 && moby_ambient.w > 0.5 ? 1 : 0;
#endif
}
