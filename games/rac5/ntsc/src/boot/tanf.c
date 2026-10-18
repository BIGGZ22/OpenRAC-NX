/* tanf -- newlib libm/math/sf_tan.c (public Newlib 1.9.0, adapted 2026-10-17). */
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

extern float __kernel_tanf(float x, float y, int iy);
extern int __ieee754_rem_pio2f(float x, float *y);

float tanf(float x) {
    float y[2], z = 0.0;
    __int32_t n, ix;
    GET_FLOAT_WORD(ix, x);
    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3f490fda) return __kernel_tanf(x, z, 1);
    /* tan(Inf or NaN) is NaN */
    else if (ix >= 0x7f800000)
        return x - x; /* NaN */
    /* argument reduction needed */
    else {
        n = __ieee754_rem_pio2f(x, y);
        return __kernel_tanf(y[0], y[1], 1 - ((n & 1) << 1)); /*   1 -- n even
                                                                 -1 -- n odd */
    }
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_tan.c
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
