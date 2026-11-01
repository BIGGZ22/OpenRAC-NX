// SPDX-License-Identifier: GPL-3.0-or-later
#version 460
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 texcoord;
layout(std140,binding=0) uniform Scene { mat4 transform; uvec4 material; };
layout(binding=0) uniform sampler2D diffuse;
layout(location=0) out vec4 color;
void main() {
    color = vertex_color;
    if (material.x != 0u) color *= texture(diffuse,texcoord);
}
