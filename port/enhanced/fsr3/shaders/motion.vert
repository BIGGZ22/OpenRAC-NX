#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Current and previous displayed poses use the same skinning operation as the world mesh.
invariant gl_Position;
layout(location=0) in vec3 position;
layout(location=2) in vec2 uv;
layout(location=3) in vec4 colour;
layout(location=10) in uvec4 joints;
layout(location=11) in vec4 weights;
layout(location=12) in float light_slot;
uniform mat4 raster;
uniform mat4 current_view;
uniform mat4 previous_view;
uniform mat4 current_model;
uniform mat4 previous_model;
uniform vec2 palettes;
uniform float reactive;
uniform int exclude_glow;
uniform float instance_alpha;
uniform int motion_instances;
uniform int motion_first;
uniform usamplerBuffer tex_T3;
uniform sampler2D tex_T1;
uniform sampler2D tex_T2;
out vec2 v_uv;
out float v_alpha;
out vec4 v_current;
out vec4 v_previous;
flat out int v_glow;
flat out float v_instance_alpha;
flat out float v_reactive;
// Palettes store four columns per joint, packed into rows of 1024 texels.
mat4 joint_matrix(sampler2D source, int index) {
    int texel = index * 4;
    ivec2 at = ivec2(texel % 1024, texel / 1024);
    return mat4(texelFetch(source, at, 0), texelFetch(source, at + ivec2(1,0), 0),
                texelFetch(source, at + ivec2(2,0), 0), texelFetch(source, at + ivec2(3,0), 0));
}
// Rigid instances keep their bind pose; animated ones use the displayed joint palette.
vec4 posed(sampler2D source, int base) {
    vec4 p = vec4(position, 1);
    float total = weights.x + weights.y + weights.z + weights.w;
    if (base >= 0 && total > 0) {
        mat4 skin = weights.x * joint_matrix(source, base + int(joints.x))
                  + weights.y * joint_matrix(source, base + int(joints.y))
                  + weights.z * joint_matrix(source, base + int(joints.z))
                  + weights.w * joint_matrix(source, base + int(joints.w));
        p = vec4((skin * p).xyz, 1);
    }
    return p;
}
void main() {
    mat4 current = current_model;
    mat4 previous = previous_model;
    vec2 joints_at = palettes;
    float reject_history = reactive;
    int glow = exclude_glow;
    v_instance_alpha = instance_alpha;
    if (motion_instances != 0) {
        // Each record holds eight matrix columns and one opacity/palette/flag texel.
        int at = (motion_first + gl_InstanceID) * 9;
        current = mat4(uintBitsToFloat(texelFetch(tex_T3, at)),
                       uintBitsToFloat(texelFetch(tex_T3, at + 1)),
                       uintBitsToFloat(texelFetch(tex_T3, at + 2)),
                       uintBitsToFloat(texelFetch(tex_T3, at + 3)));
        previous = mat4(uintBitsToFloat(texelFetch(tex_T3, at + 4)),
                        uintBitsToFloat(texelFetch(tex_T3, at + 5)),
                        uintBitsToFloat(texelFetch(tex_T3, at + 6)),
                        uintBitsToFloat(texelFetch(tex_T3, at + 7)));
        uvec4 parameters = texelFetch(tex_T3, at + 8);
        joints_at = uintBitsToFloat(parameters.xy);
        v_instance_alpha = uintBitsToFloat(parameters.z);
        reject_history = float(parameters.w & 1u);
        glow = int((parameters.w >> 1u) & 1u);
    }
    v_reactive = reject_history;
    vec4 world = current * posed(tex_T1, int(joints_at.x));
    gl_Position = raster * world;
    v_current = current_view * world;
    // Newly visible objects have no previous joint range and must never fetch from it.
    vec4 previous_world = reject_history > 0.5 ? world
        : previous * posed(tex_T2, int(joints_at.y));
    v_previous = previous_view * previous_world;
    v_uv = uv;
    v_alpha = colour.a;
    v_glow = glow != 0 && light_slot < -1.5 ? 1 : 0;
}
