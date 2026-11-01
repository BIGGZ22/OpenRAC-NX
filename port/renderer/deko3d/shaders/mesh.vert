// SPDX-License-Identifier: GPL-3.0-or-later
#version 460
layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec2 uv;
layout(std140,binding=0) uniform Scene { mat4 transform; uvec4 material; };
layout(location=0) out vec4 vertex_color;
layout(location=1) out vec2 texcoord;
void main() {
    gl_Position = transform * vec4(position,1.0);
    vertex_color = color; texcoord = uv;
}
