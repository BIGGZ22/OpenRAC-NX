#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2,gl_VertexID & 2);
    gl_Position = vec4(p*2.0-1.0,0.0,1.0);
}
