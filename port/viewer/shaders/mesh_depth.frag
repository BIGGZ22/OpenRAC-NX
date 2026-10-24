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
in vec3 v_surface_position;
layout(location = 0) out vec4 surface_normal;
#endif
void main() {
#ifdef OPENRAC_DEPTH_NORMALS
    // Raster derivatives orient the smooth normal without sampling or reconstructing depth normals.
    vec3 geometric = cross(dFdx(v_surface_position), dFdy(v_surface_position));
#endif
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
    if (!any(isnan(v_surface_normal)) && !isinf(squared) && squared > 1e-12
        && dot(geometric, geometric) > 1e-12) {
        vec3 normal = orient_shading_normal(v_surface_normal, geometric, -v_surface_position);
        surface_normal = vec4(normal * 0.5 + 0.5, 1);
    }
#endif
}
