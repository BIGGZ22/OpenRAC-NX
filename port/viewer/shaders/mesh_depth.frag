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
#ifdef OPENRAC_DEPTH_NORMALS
in vec3 v_surface_normal;
layout(location = 0) out vec4 surface_normal;
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
#ifdef OPENRAC_DEPTH_NORMALS
    // Invalid authored normals retain the neutral fallback instead of propagating NaNs.
    float squared = dot(v_surface_normal, v_surface_normal);
    surface_normal = vec4(0);
    if (!any(isnan(v_surface_normal)) && !isinf(squared) && squared > 1e-12) {
        surface_normal = vec4(v_surface_normal * inversesqrt(squared) * 0.5 + 0.5, 1);
    }
#endif
}
