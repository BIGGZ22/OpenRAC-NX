#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The direct renderer's vertices arrive already in device coordinates (the
// CPU applied XYOFFSET and the screen size); colours come as the chip's bytes.

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 stq;
layout(location = 2) in vec4 colour;  // normalised bytes
layout(location = 3) in float fog;

out vec3 v_stq;
out vec4 v_colour;  // the chip's units: 0..255, 0x80 = 1.0 when modulating
out float v_fog;

// A quarter of a pixel of the target, in device units (x on, y down): the chip samples a pixel at
// its integer coordinates, GL at its centre (DirectRenderer, convert).
uniform vec2 pixel_offset;

void main() {
    gl_Position = vec4(position.xy + pixel_offset, position.z, 1.0);
    v_stq = stq;
    v_colour = colour * 255.0;
    v_fog = fog;
}
