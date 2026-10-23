/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Going Commando's VU0 scalar trigonometry.
 *
 * 0x00283180 and 0x00283198 move their float argument into VU0 and call the
 * microprogram that the VIF MPG packet at 0x0010E9EC loads at 0xC80 (94
 * instruction pairs): entry 0xC80 adds pi/2 (cosine) and runs on into entry
 * 0xC90, the sine. It folds the angle into [-pi/2, pi/2] with pi - x and
 * -pi - x, then sums the odd series x + c3 x^3 + c5 x^5 + c7 x^7 + c9 x^9 with
 * the constants the program loads through its I register (0xCF0..0xD08).
 * The constants are read from the packet the game itself loaded; the sum is
 * the port's own order of operations, not a matching claim
 * (common/lib/vu0_sine.h).
 */
#include "game/common/lib/vu0_sine.h"

/* The microprogram's constants: pi/2 at 0xC80, pi at 0xC90, -pi at 0xC98 and
 * c3..c9 at 0xCF0..0xD08. The packet's code starts one word after its MPG tag. */
static const openrac_vu0_sine_program program = {
    .code = 0x0010E9F0u,
    .load = 0xC80u,
    .cosine_entry = 0xC80u,
    .sine_entry = 0xC90u,
    .minus_pi = 0xC98u,
    .series = {0xCF0u, 0xCF8u, 0xD00u, 0xD08u},
    .order = OPENRAC_VU0_SINE_NESTED,
};

float func_00283180(float angle) {
    return openrac_vu0_cosine(&program, angle);
}

float func_00283198(float angle) {
    return openrac_vu0_sine(&program, angle);
}
