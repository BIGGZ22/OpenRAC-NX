/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * openrac/ps2float.h, the EE FPU's arithmetic for C: saturation where the host has infinities and
 * NaNs, results cut toward zero, the adder's dropped bits, zeros for denormals, the divide's and
 * the reciprocal root's edges, and comparisons by sign and magnitude. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "openrac/ps2float.h"

static int failures;

#define CHECK(c)                                                                                   \
    do {                                                                                           \
        if (!(c)) {                                                                                \
            fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #c);                  \
            failures++;                                                                            \
        }                                                                                          \
    } while (0)

static float f(uint32_t bits) {
    float x;
    memcpy(&x, &bits, 4);
    return x;
}

static uint32_t u(float x) {
    uint32_t bits;
    memcpy(&bits, &x, 4);
    return bits;
}

/* A float of the console's: positive, exponent 1..254 (no zero or denormal, no host infinity). */
static uint32_t state = 12345;
static float some_float(int low_exponent, int high_exponent) {
    state = state * 1103515245u + 12345u;
    uint32_t mantissa = (state >> 4) & 0x7FFFFFu;
    state = state * 1103515245u + 12345u;
    uint32_t span = (uint32_t)(high_exponent - low_exponent + 1);
    int exponent = low_exponent + (int)((state >> 16) % span);
    return f((uint32_t)exponent << 23 | mantissa);
}

/* The next float above a positive one. */
static double next_up(float x) {
    return (double)f(u(x) + 1);
}

static void saturation(void) {
    const float big = f(0x7F000000u);       /* 2^127 */
    /* An overflow is +-0x7FFFFFFF, the largest value with exponent 255, not an infinity. */
    CHECK(u(openrac_ps2_mul(big, big)) == 0x7FFFFFFFu);
    CHECK(u(openrac_ps2_mul(big, -big)) == 0xFFFFFFFFu);
    CHECK(u(openrac_ps2_add(f(0x7F7FFFFFu), f(0x7F7FFFFFu))) == 0x7FFFFFFFu);
    CHECK(u(openrac_ps2_sub(f(0xFF7FFFFFu), f(0x7F7FFFFFu))) == 0xFFFFFFFFu);
    /* Exponent 255 is an ordinary number: the host's infinity and NaN patterns compute. */
    CHECK(u(openrac_ps2_mul(f(0x7F800000u), 0.5f)) == 0x7F000000u);
    CHECK(u(openrac_ps2_mul(f(0x7FC00000u), 0.5f)) == 0x7F400000u);
    /* one place of 2^128's binade: 2^-23 * 2^128 */
    CHECK(u(openrac_ps2_sub(f(0x7F800001u), f(0x7F800000u))) == 0x74000000u);
    /* An underflow is a zero of the product's sign. */
    CHECK(u(openrac_ps2_mul(f(0x0D800000u), f(0x0D800000u))) == 0x00000000u);
    CHECK(u(openrac_ps2_mul(f(0x8D800000u), f(0x0D800000u))) == 0x80000000u);
}

static void cut_toward_zero(void) {
    int i;
    /* 1/3: the host's nearest is 0x3EAAAAAB, the EE cuts to 0x3EAAAAAA; -1/3 the same magnitude. */
    CHECK(u(openrac_ps2_div(1.0f, 3.0f)) == 0x3EAAAAAAu);
    CHECK(u(openrac_ps2_div(-1.0f, 3.0f)) == 0xBEAAAAAAu);
    /* 1.1 * 1.1: the exact product is cut, not rounded. */
    CHECK(u(openrac_ps2_mul(f(0x3F8CCCCDu), f(0x3F8CCCCDu))) == 0x3F9AE148u);
    /* cvt.s.w cuts an integer of more than 24 bits: 2^24 + 3 is 2^24 + 2 to the nearest. */
    CHECK(u(openrac_ps2_itof(16777219)) == 0x4B800001u);
    CHECK(u(openrac_ps2_itof(-16777219)) == 0xCB800001u);
    CHECK(u(openrac_ps2_itof(0)) == 0u);
    CHECK(openrac_ps2_itof(-7) == -7.0f);
    /* cvt.w.s: toward zero, saturating by the sign; exponent 255 is a number, exponent 0 zero. */
    CHECK(openrac_ps2_ftoi(f(0x3FF33333u)) == 1);
    CHECK(openrac_ps2_ftoi(f(0xBFF33333u)) == -1);
    CHECK(openrac_ps2_ftoi(3e9f) == 2147483647);
    CHECK(openrac_ps2_ftoi(-3e9f) == -2147483647 - 1);
    CHECK(openrac_ps2_ftoi(openrac_ps2_div(1.0f, 0.0f)) == 2147483647);
    CHECK(openrac_ps2_ftoi(openrac_ps2_div(-1.0f, 0.0f)) == -2147483647 - 1);
    CHECK(openrac_ps2_ftoi(f(0x00400000u)) == 0);
    /* Every product and quotient of positive normal numbers lies at or below the exact value,
       less than one place under it (the multiplier's rare last-bit deviations are not modelled). */
    for (i = 0; i < 20000; i++) {
        const float a = some_float(64, 190), b = some_float(64, 190);
        const double p = (double)a * (double)b, q = (double)a / (double)b;
        const float mp = openrac_ps2_mul(a, b), mq = openrac_ps2_div(a, b);
        CHECK((double)mp <= p && p < next_up(mp));
        CHECK((double)mq <= q && q < next_up(mq));
    }
}

static void adder(void) {
    const float small = f(0x33C00000u);     /* 1.5 * 2^-24 */
    /* The adder drops the smaller operand's bits more than one place below the larger one's last
       bit before it adds: 1 - 1.5 * 2^-24 is 1 - 2^-24 here, where the exact difference cut toward
       zero (the host rounding toward zero) is 1 - 2^-23. */
    CHECK(u(openrac_ps2_sub(1.0f, small)) == 0x3F7FFFFFu);
    CHECK(u(openrac_ps2_add(1.0f, small)) == 0x3F800000u);
    /* Exponents 25 or more apart: the larger operand, unchanged. */
    CHECK(u(openrac_ps2_add(1.0f, f(0x33000000u))) == 0x3F800000u);
    CHECK(u(openrac_ps2_sub(1.0f, f(0x33000000u))) == 0x3F800000u);
    /* x - x is +0, -0 + -0 is -0. */
    CHECK(u(openrac_ps2_sub(f(0x3F9AE148u), f(0x3F9AE148u))) == 0u);
    CHECK(u(openrac_ps2_add(-0.0f, -0.0f)) == 0x80000000u);
}

static void denormals_are_zero(void) {
    const float tiny = f(0x00000001u), negative_tiny = f(0x80400000u);
    CHECK(u(openrac_ps2_add(tiny, 1.0f)) == 0x3F800000u);
    CHECK(u(openrac_ps2_add(tiny, tiny)) == 0u);
    CHECK(u(openrac_ps2_mul(negative_tiny, 2.0f)) == 0x80000000u);
    CHECK(u(openrac_ps2_div(tiny, 2.0f)) == 0u);
    CHECK(openrac_ps2_eq(tiny, 0.0f));
    CHECK(openrac_ps2_eq(negative_tiny, 0.0f));
    CHECK(!openrac_ps2_lt(negative_tiny, tiny));
}

static void divide_and_roots(void) {
    int i;
    /* A divisor of zero (or a denormal): +-0x7FFFFFFF, the operands' signs XORed. */
    CHECK(u(openrac_ps2_div(1.0f, 0.0f)) == 0x7FFFFFFFu);
    CHECK(u(openrac_ps2_div(-1.0f, 0.0f)) == 0xFFFFFFFFu);
    CHECK(u(openrac_ps2_div(1.0f, -0.0f)) == 0xFFFFFFFFu);
    CHECK(u(openrac_ps2_div(0.0f, 0.0f)) == 0x7FFFFFFFu);
    CHECK(u(openrac_ps2_div(2.0f, f(0x00000010u))) == 0x7FFFFFFFu);
    /* A dividend of zero over a number: a zero of the signs XORed. */
    CHECK(u(openrac_ps2_div(0.0f, -4.0f)) == 0x80000000u);
    /* sqrt.s: the root of the magnitude, cut. */
    CHECK(openrac_ps2_sqrt(4.0f) == 2.0f);
    CHECK(openrac_ps2_sqrt(-4.0f) == 2.0f);
    CHECK(u(openrac_ps2_sqrt(f(0x00000003u))) == 0u);
    CHECK(u(openrac_ps2_sqrt(f(0x7FFFFFFFu))) == 0x5FB504F2u);
    /* rsqrt.s: a over the cut root of |b|, cut again; b of zero saturates by a's sign. */
    CHECK(openrac_ps2_rsqrt(1.0f, 4.0f) == 0.5f);
    CHECK(openrac_ps2_rsqrt(1.0f, -4.0f) == 0.5f);
    CHECK(openrac_ps2_rsqrt(-3.0f, 9.0f) == -1.0f);
    CHECK(u(openrac_ps2_rsqrt(1.0f, 0.0f)) == 0x7FFFFFFFu);
    CHECK(u(openrac_ps2_rsqrt(-1.0f, 0.0f)) == 0xFFFFFFFFu);
    CHECK(u(openrac_ps2_rsqrt(1.0f, -0.0f)) == 0x7FFFFFFFu);
    CHECK(u(openrac_ps2_rsqrt(1.0f, f(0x00000001u))) == 0x7FFFFFFFu);
    CHECK(u(openrac_ps2_rsqrt(0.0f, 2.0f)) == 0u);
    /* 1 / sqrt(2): the root cut first (0x3FB504F3), then the quotient (0x3F3504F3; the host's
       1.0f / sqrtf(2.0f) is 0x3F3504F4, its nearest). */
    CHECK(u(openrac_ps2_sqrt(2.0f)) == 0x3FB504F3u);
    CHECK(u(openrac_ps2_rsqrt(1.0f, 2.0f)) == 0x3F3504F3u);
    for (i = 0; i < 20000; i++) {
        const float x = some_float(1, 254);
        const float r = openrac_ps2_sqrt(x);
        CHECK((double)r * (double)r <= (double)x && (double)x < next_up(r) * next_up(r));
        CHECK(u(openrac_ps2_rsqrt(1.0f, x)) == u(openrac_ps2_div(1.0f, r)));
    }
}

static void comparisons(void) {
    const float q = openrac_ps2_div(1.0f, 0.0f);    /* 0x7FFFFFFF, a NaN to the host */
    CHECK(openrac_ps2_lt(1.0f, q));
    CHECK(openrac_ps2_gt(q, 1.0f));
    CHECK(openrac_ps2_ge(q, q));
    CHECK(openrac_ps2_eq(q, q));
    CHECK(openrac_ps2_lt(openrac_ps2_div(-1.0f, 0.0f), -1.0f));
    CHECK(openrac_ps2_lt(f(0x7F800000u), q));
    CHECK(openrac_ps2_eq(-0.0f, 0.0f));
    CHECK(!openrac_ps2_lt(-0.0f, 0.0f));
    CHECK(openrac_ps2_le(-0.0f, 0.0f));
    CHECK(openrac_ps2_lt(-2.0f, -1.0f));
    CHECK(!openrac_ps2_gt(-2.0f, -1.0f));
    CHECK(openrac_ps2_ge(-1.0f, -2.0f));
    CHECK(!openrac_ps2_eq(1.0f, f(0x3F800001u)));
}

int main(void) {
    saturation();
    cut_toward_zero();
    adder();
    denormals_are_zero();
    divide_and_roots();
    comparisons();
    if (failures != 0) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    return 0;
}
