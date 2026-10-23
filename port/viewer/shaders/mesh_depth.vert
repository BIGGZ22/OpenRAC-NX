#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Match mesh.vert's position/alpha arithmetic; omit normals, lighting and fog.
invariant gl_Position;
layout(location = 0) in vec3 position;
layout(location = 4) in mat4 model;
layout(location = 9) in float palette;
layout(location = 10) in uvec4 joints;
layout(location = 11) in vec4 weights;
layout(location = 14) in vec4 moby_light;
uniform mat4 view_projection;
uniform float picture_scale;
uniform int skinning;
uniform sampler2D palette_texture;
#ifndef OPENRAC_DEPTH_SOLID
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 colour;
layout(location = 8) in vec4 tint;
layout(location = 12) in float light_slot;
layout(location = 13) in float lights;
layout(location = 15) in vec4 moby_ambient;
uniform sampler2D light_texture;
out vec2 v_uv;
out float v_alpha;
flat out int v_glow;
#endif
const int kPaletteWidth = 1024;

mat4 joint_matrix(int index) {
    int texel = 4 * index;
    ivec2 at = ivec2(texel % kPaletteWidth, texel / kPaletteWidth);
    return mat4(
        texelFetch(palette_texture, at, 0),
        texelFetch(palette_texture, at + ivec2(1, 0), 0),
        texelFetch(palette_texture, at + ivec2(2, 0), 0),
        texelFetch(palette_texture, at + ivec2(3, 0), 0)
    );
}

void main() {
    vec4 p = vec4(position, 1.0);
    float total = weights.x + weights.y + weights.z + weights.w;
    if (skinning != 0 && palette >= 0.0 && total > 0.0) {
        int base = int(palette + 0.5);
        mat4 skin = weights.x * joint_matrix(base + int(joints.x))
                    + weights.y * joint_matrix(base + int(joints.y))
                    + weights.z * joint_matrix(base + int(joints.z))
                    + weights.w * joint_matrix(base + int(joints.w));
        p = vec4((skin * p).xyz, 1.0);
    }
    vec4 world = model * p;
    gl_Position = view_projection * world;
    bool in_picture = moby_light.w > 1.5 && picture_scale < 1.0;
    gl_ClipDistance[0] = in_picture ? picture_scale * gl_Position.w - gl_Position.x : 1.0;
    gl_ClipDistance[1] = in_picture ? picture_scale * gl_Position.w + gl_Position.x : 1.0;
#ifndef OPENRAC_DEPTH_SOLID
    v_uv = uv;
    v_alpha = colour.a * tint.a;
    if (lights >= 0.0 && light_slot >= 0.0) {
        int texel = int(lights + 0.5) + int(light_slot + 0.5);
        v_alpha = texelFetch(light_texture, ivec2(texel % 1024, texel / 1024), 0).a
                  * (255.0 / 128.0) * tint.a;
    }
    if (abs(moby_light.w) > 0.5) {
        v_alpha = colour.a * tint.a;
    }
    v_glow = abs(moby_light.w) > 0.5 && light_slot < -1.5 && moby_ambient.w > 0.5 ? 1 : 0;
#endif
}
