/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's VU0 scalar trigonometry.
 *
 * 0x00283180 and 0x00283198 move their float argument into VU0 and call the
 * microprogram that the VIF MPG packet at 0x0010E9EC loads at 0xC80 (94
 * instruction pairs): entry 0xC80 adds pi/2 (cosine) and runs on into entry
 * 0xC90, the sine. It folds the angle into [-pi/2, pi/2] with pi - x and
 * -pi - x, then sums the odd series x + c3 x^3 + c5 x^5 + c7 x^7 + c9 x^9 with
 * the constants the program loads through its I register (0xCF0..0xD08).
 * The constants are read from the packet the game itself loaded; the sum is
 * the port's own order of operations, not a matching claim.
 */
#include <stdint.h>
#include <string.h>

#include "openrac/guest.h"

/* The lower word of the instruction pair at microprogram address ENTRY: the
 * I-register immediate of a LOI pair. The packet's code starts one word after
 * its MPG tag. */
static float program_constant(uint32_t entry) {
    float value;
    memcpy(&value, G(0x0010E9F0u + (entry - 0xC80u)), sizeof value);
    return value;
}

static float vu0_sine(float angle) {
    const float pi = program_constant(0xC90u);
    const float upper = pi - angle;
    const float lower = program_constant(0xC98u) - angle;
    float x = angle < upper ? angle : upper;
    x = x > lower ? x : lower;
    const float x2 = x * x;
    const float c3 = program_constant(0xCF0u);
    const float c5 = program_constant(0xCF8u);
    const float c7 = program_constant(0xD00u);
    const float c9 = program_constant(0xD08u);
    return x + x * x2 * (c3 + x2 * (c5 + x2 * (c7 + x2 * c9)));
}

float func_00283180(float angle) {
    return vu0_sine(angle + program_constant(0xC80u));
}

float func_00283198(float angle) {
    return vu0_sine(angle);
}
