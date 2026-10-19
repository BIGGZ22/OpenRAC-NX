#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
layout(location=0) in vec3 position;
uniform mat4 view_projection;
void main() {
    gl_Position = view_projection * vec4(position,1.0);
    // Keep the far cap closed at the reversed-depth far plane. Near clipping
    // is safe with depth-fail counting, including a camera inside the prism.
    gl_Position.z = max(gl_Position.z,-gl_Position.w);
}
