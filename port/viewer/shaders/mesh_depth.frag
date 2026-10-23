#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Opaque variant has no texture fetch or discard. Cutout/moby depth exactly
// matches mesh.frag's solid pass: glow excluded, combined alpha >= 0.99.
#ifndef OPENRAC_DEPTH_SOLID
in vec2 v_uv;
in float v_alpha;
flat in int v_glow;
uniform sampler2D tex_T0;
uniform vec4 material_colour;
uniform int cutout;
#endif
void main() {
#ifndef OPENRAC_DEPTH_SOLID
    if (v_glow != 0) {
        discard;
    }
    if (cutout != 0 && texture(tex_T0, v_uv).a * v_alpha * material_colour.a < 0.99) {
        discard;
    }
#endif
}
