/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * A game's VU0 scalar sine and cosine (vu0_sine.h): the constants read from
 * the game's own microprogram, the fold and the two orders of the series. The
 * games' C is compiled without fused multiply-add (cmake/Games.cmake), and so
 * is this, so each order rounds as the game's port always has. */
#include "game/common/lib/vu0_sine.h"

#include <stdint.h>
#include <string.h>

#include "openrac/guest.h"

/* Returns the constant of the instruction pair at microprogram address
 * `entry`: the I-register immediate of a LOI pair, its lower word. */
static float program_constant(const openrac_vu0_sine_program* program, uint32_t entry) {
    float value;
    memcpy(&value, G(program->code + (entry - program->load)), sizeof value);
    return value;
}

/* Returns x + c3 x^3 + c5 x^5 + c7 x^7 + c9 x^9, nested in x^2. */
static float series_nested(float x, float c3, float c5, float c7, float c9) {
    const float x2 = x * x;
    return x + x * x2 * (c3 + x2 * (c5 + x2 * (c7 + x2 * c9)));
}

/* Returns x + c3 x^3 + c5 x^5 + c7 x^7 + c9 x^9 as two halves joined by x^4:
 * the products of x and the constants first, then the sums. */
static float series_halves(float x, float c3, float c5, float c7, float c9) {
    const float x2 = x * x;
    const float x4 = x2 * x2;
    const float p7 = x * c7;
    const float p9 = x * c9;
    const float p3 = x * c3;
    float accumulator = x * c5;
    accumulator = accumulator + p7 * x2;
    const float high = accumulator + p9 * x4;
    accumulator = x + p3 * x2;
    return accumulator + high * x4;
}

float openrac_vu0_sine(const openrac_vu0_sine_program* program, float angle) {
    /* The fold: pi - x above pi/2, -pi - x below -pi/2. */
    const float pi = program_constant(program, program->sine_entry);
    const float upper = pi - angle;
    /* A program with no -pi of its own subtracts from pi negated. */
    const float minus_pi = program->minus_pi ? program_constant(program, program->minus_pi) : -pi;
    const float lower = minus_pi - angle;
    float x = angle < upper ? angle : upper;
    x = x > lower ? x : lower;

    const float c3 = program_constant(program, program->series[0]);
    const float c5 = program_constant(program, program->series[1]);
    const float c7 = program_constant(program, program->series[2]);
    const float c9 = program_constant(program, program->series[3]);
    /* Each game's port keeps the order it has always summed in. */
    return program->order == OPENRAC_VU0_SINE_HALVES ? series_halves(x, c3, c5, c7, c9)
                                                     : series_nested(x, c3, c5, c7, c9);
}

float openrac_vu0_cosine(const openrac_vu0_sine_program* program, float angle) {
    return openrac_vu0_sine(program, angle + program_constant(program, program->cosine_entry));
}
