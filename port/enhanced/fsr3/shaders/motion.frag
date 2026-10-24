#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Opaque velocities point from current unjittered UV to previous unjittered UV, in render pixels.
in vec2 v_uv;
in float v_alpha;
in vec4 v_current;
in vec4 v_previous;
flat in int v_glow;
uniform sampler2D tex_T0;
uniform float instance_alpha;
uniform float material_alpha;
uniform float reactive;
uniform vec2 render_size;
layout(location=0) out vec4 velocity;
void main() {
    if (v_glow != 0 || texture(tex_T0, v_uv).a * v_alpha * instance_alpha * material_alpha < 0.99) {
        discard;
    }
    bool valid = v_previous.w > 0 && v_current.w > 0;
    vec2 motion = valid ? (v_previous.xy / v_previous.w - v_current.xy / v_current.w)
                         * 0.5 * render_size : vec2(0);
    velocity = vec4(motion, valid ? reactive : 1, 0);
}
