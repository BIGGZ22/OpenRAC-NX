/* floorf -- newlib libm/math/sf_floor.c (public Newlib 1.9.0, adapted 2026-10-17). */
typedef int __int32_t;
typedef unsigned int __uint32_t;

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

#define SET_FLOAT_WORD(d, i)                                                   \
    do {                                                                       \
        ieee_float_shape_type sf_u;                                            \
        sf_u.word = (i);                                                       \
        (d) = sf_u.value;                                                      \
    } while (0)

static const float huge = 1.0e30;

float floorf(float x) {
    __int32_t i0, j0;
    __uint32_t i;
    GET_FLOAT_WORD(i0, x);
    j0 = ((i0 >> 23) & 0xff) - 0x7f;
    if (j0 < 23) {
        if (j0 < 0) { /* raise inexact if x != 0 */
            if (huge + x > (float)0.0) { /* return 0*sign(x) if |x|<1 */
                if (i0 >= 0) {
                    i0 = 0;
                } else if ((i0 & 0x7fffffff) != 0) {
                    i0 = 0xbf800000;
                }
            }
        } else {
            i = (0x007fffff) >> j0;
            if ((i0 & i) == 0) return x; /* x is integral */
            if (huge + x > (float)0.0) { /* raise inexact flag */
                if (i0 < 0) i0 += (0x00800000) >> j0;
                i0 &= (~i);
            }
        }
    } else {
        if (j0 == 0x80) return x + x; /* inf or NaN */
        else return x;                /* x is integral */
    }
    SET_FLOAT_WORD(x, i0);
    return x;
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_floor.c
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
