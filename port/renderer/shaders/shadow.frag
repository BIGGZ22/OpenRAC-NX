#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
uniform float opacity;
out vec4 frag_colour;
void main() { frag_colour = vec4(0.0,0.0,0.0,opacity); }
