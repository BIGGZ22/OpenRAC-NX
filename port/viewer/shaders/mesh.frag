#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Texture times vertex colour, alpha-tested for cutout materials, with a
// fixed sun so shapes read where the export has no lighting. The editor's PNGs
// carry alpha from 0 to 255 (it doubles the chip's 0..0x80), so the cutout
// threshold is plain 0.5. A vertex colour of 1.0 is the GS's 0x80: the product
// is the GS's MODULATE, (texel x colour) >> 7, clamped like the chip's.

in vec2 v_uv;
in vec4 v_colour;
in vec3 v_normal;
flat in int v_lit;
flat in int v_glow;
noperspective in float v_fog;

uniform sampler2D tex_T0;
uniform vec4 material_colour;
uniform int cutout;
uniform int lighting;
uniform int glow_pass;  // mobys: 0 the solid faces, 1 the glow faces (blended)
// Cutout materials in two passes, as the GS draws them (alpha test GEQUAL 0x80, failing pixels
// written as colour only): 0 the opaque texels, with depth; 1 the translucent ones, blended.
uniform int translucent_pass;
// Mobys MobyProc blends (an explosion's flash, a fading pickup): every face and texel, its alpha
// the texel's times the moby's (the tint), blended by the pass.
uniform int effect_pass;
// The metal pass: the chrome or glass texel times the vertex colour, its alpha the texel's times the
// shine, blended over the moby (mesh.vert; ALPHA 0x44 as the moby's own).
uniform int metal_pass;
uniform vec4 fog_colour;  // FOGCOL; w = 1: fog on

layout(location = 0) out vec4 out_colour;

// From above and to one side, in game axes (Z up).
const vec3 kSun = vec3(0.36, 0.48, 0.80);

void main() {
    if (metal_pass != 0) {
        vec4 m = texture(tex_T0, v_uv) * v_colour;
        vec3 mrgb = min(m.rgb, vec3(1.0));
        if (fog_colour.w > 0.5) {
            mrgb = mix(fog_colour.rgb, mrgb, v_fog);
        }
        out_colour = vec4(mrgb, clamp(m.a, 0.0, 1.0));
        return;
    }
    if (effect_pass == 0 && v_glow != glow_pass) {
        discard;
    }
    vec4 c = texture(tex_T0, v_uv) * v_colour * material_colour;
    if (effect_pass != 0) {
        vec3 e = min(c.rgb, vec3(1.0));
        if (fog_colour.w > 0.5) {
            e = mix(fog_colour.rgb, e, v_fog);
        }
        out_colour = vec4(e, clamp(c.a, 0.0, 1.0));
        return;
    }
    // The glow pass blends every texel at its alpha (the Gadgetron hologram's lettering is mostly
    // part-transparent texels); only the solid pass leaves a cutout's part-transparent texels out.
    if (cutout != 0 && translucent_pass == 0 && glow_pass == 0 && c.a < 0.99) {
        discard;
    }
    if (translucent_pass != 0 && (cutout == 0 || c.a >= 0.99 || c.a < 0.02)) {
        discard;
    }
    float light = 1.0;
    if (lighting != 0 && v_lit == 0) {
        // Double-sided: a face lit from behind is lit too.
        light = 0.55 + 0.45 * abs(dot(normalize(v_normal), kSun));
    }
    vec3 rgb = min(c.rgb * light, vec3(1.0));
    // The GS's fog, after the texture: RGB only.
    if (fog_colour.w > 0.5) {
        rgb = mix(fog_colour.rgb, rgb, v_fog);
    }
    out_colour = vec4(rgb, v_glow != 0 || translucent_pass != 0 ? clamp(c.a, 0.0, 1.0) : 1.0);
}
