/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Deadlocked VU0 scalar trigonometry, reconstructed from its retail patch.
 * The VIF MPG packet at 0x0010F1E0 loads 148 instruction pairs at 0xB10.
 * Entry 0xC78 shifts x by pi/2; 0xC88 evaluates the sine polynomial.
 * This is a native implementation, not a matching claim
 * (common/lib/vu0_sine.h).
 */
#include "game/common/lib/vu0_sine.h"

/* The patch's constants: pi/2 at 0xC78, pi at 0xC88, and c7, c9, c3, c5 at
 * 0xCC0, 0xCC8, 0xCD0, 0xCE0. It has no -pi of its own. Its code starts 16
 * bytes into the packet. */
static const openrac_vu0_sine_program program = {
    .code = 0x0010F1F0u,
    .load = 0xB10u,
    .cosine_entry = 0xC78u,
    .sine_entry = 0xC88u,
    .minus_pi = 0,
    .series = {0xCD0u, 0xCE0u, 0xCC0u, 0xCC8u},
    .order = OPENRAC_VU0_SINE_HALVES,
};

float func_00453928(float angle) {
    return openrac_vu0_cosine(&program, angle);
}

float func_00453940(float angle) {
    return openrac_vu0_sine(&program, angle);
}
