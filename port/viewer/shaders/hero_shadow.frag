#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef OPENRAC_SHADOW_SOLID
in vec2 v_uv;
in vec4 v_colour;
flat in int v_glow;
uniform sampler2D tex_T0;
uniform vec4 material_colour;
uniform int cutout;
#endif
void main() {
#ifndef OPENRAC_SHADOW_SOLID
    float alpha = v_colour.a * material_colour.a;
    if (v_glow != 0 || alpha < 0.5 || (cutout != 0 && texture(tex_T0, v_uv).a * alpha < 0.5)) {
        discard;
    }
#endif
}
