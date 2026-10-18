/* sin -- newlib libm/math/s_sin.c (public Newlib 1.9.0, adapted 2026-10-17). */
typedef int __int32_t;
typedef unsigned int __uint32_t;

typedef union {
    double value;
    struct {
        __uint32_t lsw;
        __uint32_t msw;
    } parts;
} ieee_double_shape_type;

#define GET_HIGH_WORD(i, d)                                                    \
    do {                                                                       \
        ieee_double_shape_type gh_u;                                           \
        gh_u.value = (d);                                                      \
        (i) = gh_u.parts.msw;                                                  \
    } while (0)

extern double __kernel_sin(double x, double y, int iy);
extern double __kernel_cos(double x, double y);
extern int __ieee754_rem_pio2(double x, double *y);

double sin(double x) {
    double y[2], z = 0.0;
    __int32_t n, ix;

    /* High word of x. */
    GET_HIGH_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3fe921fb) return __kernel_sin(x, z, 0);

    /* sin(Inf or NaN) is NaN */
    else if (ix >= 0x7ff00000)
        return x - x;

    /* argument reduction needed */
    else {
        n = __ieee754_rem_pio2(x, y);
        switch (n & 3) {
        case 0:
            return __kernel_sin(y[0], y[1], 1);
        case 1:
            return __kernel_cos(y[0], y[1]);
        case 2:
            return -__kernel_sin(y[0], y[1], 1);
        default:
            return -__kernel_cos(y[0], y[1]);
        }
    }
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/s_sin.c
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
