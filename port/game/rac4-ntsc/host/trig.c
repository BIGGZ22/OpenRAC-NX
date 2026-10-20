/* SPDX-License-Identifier: GPL-3.0-or-later
 * Deadlocked VU0 scalar trigonometry, reconstructed from its retail patch.
 * The VIF MPG packet at 0x0010F1E0 loads 148 instruction pairs at 0xB10.
 * Entry 0xC78 shifts x by pi/2; 0xC88 evaluates the sine polynomial.
 * This is a native implementation, not a matching claim.
 */
#include <stdint.h>
#include <string.h>
#include "openrac/guest.h"

static float patch_constant(uint32_t entry) {
    float value;
    memcpy(&value, G(0x0010F1F0u + entry - 0xB10u), sizeof value);
    return value;
}

static float patch_sine(float angle) {
    const float pi = patch_constant(0xC88u);
    const float upper = pi - angle;
    const float lower = -pi - angle;
    float x = angle < upper ? angle : upper;
    x = x > lower ? x : lower;
    const float x2 = x * x;
    const float x4 = x2 * x2;
    const float p7 = x * patch_constant(0xCC0u);
    const float p9 = x * patch_constant(0xCC8u);
    const float p3 = x * patch_constant(0xCD0u);
    float accumulator = x * patch_constant(0xCE0u);
    accumulator = accumulator + p7 * x2;
    const float high = accumulator + p9 * x4;
    accumulator = x + p3 * x2;
    return accumulator + high * x4;
}

float func_00453928(float angle) {
    return patch_sine(angle + patch_constant(0xC78u));
}

float func_00453940(float angle) {
    return patch_sine(angle);
}
