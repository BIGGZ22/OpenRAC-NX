// SPDX-License-Identifier: GPL-3.0-or-later
#version 460
const vec2 points[3] = vec2[](vec2(-0.7,-0.6),vec2(0.7,-0.6),vec2(0.0,0.7));
void main() { gl_Position = vec4(points[gl_VertexID],0.0,1.0); }
