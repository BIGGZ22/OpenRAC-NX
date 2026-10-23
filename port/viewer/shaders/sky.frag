#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A shell's texel (white where untextured) times its vertex colour; the alpha
// blends it over the shells before it and the background.

in vec2 v_uv;
in vec4 v_colour;

uniform sampler2D tex_T0;

layout(location = 0) out vec4 out_colour;

// OPENRAC_UPSCALED_TEXTURE_COLOURS_SOURCE

void main() {
    out_colour = sample_upscaled_texture_colours(v_uv) * v_colour;
}
