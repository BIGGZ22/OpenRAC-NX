#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The chip's texture function, fog and alpha test, per pixel, in the chip's
// units (colour bytes 0..255, 0x80 = 1.0 when modulating; alpha 0x80 opaque).
// Blending and the depth test are GL's fixed-function state.

in vec3 v_stq;
in vec4 v_colour;
in float v_fog;

uniform sampler2D tex_T0;
uniform sampler2D tex_T1;      // with distance_field: R the glyphs' opacity, A the distance to their edge
uniform int textured;
uniform int tcc;              // take alpha from the texture
uniform int tfx;              // 0 modulate, 1 decal, 2 highlight, 3 highlight2
uniform float tex_alpha_scale;  // 1: texture alpha in the chip's units; 0.5: 0..255
uniform int fog_enable;
uniform vec3 fog_colour;      // 0..255
uniform int alpha_test;       // 0 off, else TEST.ATST (2 LESS ... 7 NOTEQUAL)
uniform float alpha_ref;      // TEST.AREF
uniform int alpha_keep_failing;  // the second pass of the double draw
uniform int distance_field;   // the texture's alpha is a distance field (text drawn sharp)

layout(location = 0) out vec4 out_colour;

bool alpha_passes(float a) {
    float value = floor(a + 0.5);
    switch (alpha_test) {
        case 2: return value < alpha_ref;
        case 3: return value <= alpha_ref;
        case 4: return value == alpha_ref;
        case 5: return value >= alpha_ref;
        case 6: return value > alpha_ref;
        case 7: return value != alpha_ref;
        default: return true;
    }
}

#ifdef OPENRAC_UPSCALED_TEXTURE_COLOURS
// OPENRAC_UPSCALED_TEXTURE_COLOURS_SOURCE
#endif

void main() {
    vec4 c = v_colour;
    if (textured != 0) {
        // S/Q, T/Q per pixel, as the chip divides.
#ifdef OPENRAC_UPSCALED_TEXTURE_COLOURS
        vec4 t = sample_upscaled_texture_colours(v_stq.xy / v_stq.z) * 255.0;
#else
        vec4 t = texture(tex_T0, v_stq.xy / v_stq.z) * 255.0;
#endif
        if (distance_field != 0) {
            // The edge from the distance field, about a pixel wide at any size; inside it, the
            // glyph's own opacity (the font fades its glyphs downwards), in the chip's units.
            vec4 f = texture(tex_T1, v_stq.xy / v_stq.z);
            float w = max(fwidth(f.a) * 0.75, 1e-4);
            t.a = smoothstep(0.5 - w, 0.5 + w, f.a) * f.r * 128.0;
        } else {
            t.a *= tex_alpha_scale;
        }
        if (tfx == 0) {
            c.rgb = c.rgb * t.rgb / 128.0;
            c.a = tcc != 0 ? c.a * t.a / 128.0 : c.a;
        } else if (tfx == 1) {
            c.rgb = t.rgb;
            c.a = tcc != 0 ? t.a : c.a;
        } else if (tfx == 2) {
            c.rgb = c.rgb * t.rgb / 128.0 + c.a;
            c.a = tcc != 0 ? c.a + t.a : c.a;
        } else {
            c.rgb = c.rgb * t.rgb / 128.0 + c.a;
            c.a = tcc != 0 ? t.a : c.a;
        }
    }
    // COLCLAMP: the chip clamps to 0..255.
    c = clamp(c, 0.0, 255.0);
    if (fog_enable != 0) {
        c.rgb = mix(fog_colour, c.rgb, v_fog);
    }
    if (alpha_test != 0 && alpha_passes(c.a) == (alpha_keep_failing != 0)) {
        discard;
    }
    // Alpha 0x80 is 1.0 for blending.
    out_colour = vec4(c.rgb / 255.0, c.a / 128.0);
}
