/* fabs -- newlib libm/math/s_fabs.c (public Newlib 1.9.0, adapted 2026-10-17).
 * fdlibm.h unreachable from the pinned cpp: the little-endian union is copied
 * verbatim. */
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

#define SET_HIGH_WORD(d, v)                                                    \
    do {                                                                       \
        ieee_double_shape_type sh_u;                                           \
        sh_u.value = (d);                                                      \
        sh_u.parts.msw = (v);                                                  \
        (d) = sh_u.value;                                                      \
    } while (0)

double fabs(double x) {
    __uint32_t high;
    GET_HIGH_WORD(high, x);
    SET_HIGH_WORD(x, high & 0x7fffffff);
    return x;
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/s_fabs.c
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
