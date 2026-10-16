#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The sky's shells, around the camera: the view without its translation, after the shell's own
// matrix as the game drew it (its turn, scale and translation: the flight's stars move by it).

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 colour;

uniform mat4 view_projection;
uniform mat4 shell;

out vec2 v_uv;
out vec4 v_colour;

void main() {
    gl_Position = view_projection * shell * vec4(position, 1.0);
    v_uv = uv;
    v_colour = colour;
}
