/* sinf -- newlib libm/math/sf_sin.c (public Newlib 1.9.0, adapted 2026-10-17). */
typedef int __int32_t;

typedef union {
    float value;
    __int32_t word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i, d)                                                   \
    do {                                                                       \
        ieee_float_shape_type gf_u;                                            \
        gf_u.value = (d);                                                      \
        (i) = gf_u.word;                                                       \
    } while (0)

extern float __kernel_sinf(float x, float y, int iy);
extern float __kernel_cosf(float x, float y);
extern int __ieee754_rem_pio2f(float x, float *y);

float sinf(float x) {
    float y[2], z = 0.0;
    __int32_t n, ix;
    GET_FLOAT_WORD(ix, x);
    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3f490fd8) return __kernel_sinf(x, z, 0);
    /* sin(Inf or NaN) is NaN */
    else if (ix >= 0x7f800000)
        return x - x;
    /* argument reduction needed */
    else {
        n = __ieee754_rem_pio2f(x, y);
        switch (n & 3) {
        case 0:
            return __kernel_sinf(y[0], y[1], 1);
        case 1:
            return __kernel_cosf(y[0], y[1]);
        case 2:
            return -__kernel_sinf(y[0], y[1], 1);
        default:
            return -__kernel_cosf(y[0], y[1]);
        }
    }
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_sin.c
 * Retain the original SunPro permission notice below. */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
