/* isnan -- newlib libm/math/s_isnan.c (public Newlib 1.9.0, adapted 2026-10-17). */
typedef int __int32_t;
typedef unsigned int __uint32_t;

typedef union {
    double value;
    struct {
        __uint32_t lsw;
        __uint32_t msw;
    } parts;
} ieee_double_shape_type;

#define EXTRACT_WORDS(ix0, ix1, d)                                             \
    do {                                                                       \
        ieee_double_shape_type ew_u;                                           \
        ew_u.value = (d);                                                      \
        (ix0) = ew_u.parts.msw;                                                \
        (ix1) = ew_u.parts.lsw;                                                \
    } while (0)

int isnan(double x) {
    __int32_t hx, lx;
    EXTRACT_WORDS(hx, lx, x);
    hx &= 0x7fffffff;
    hx |= (__uint32_t)(lx | (-lx)) >> 31;
    hx = 0x7ff00000 - hx;
    return (int)(((__uint32_t)(hx)) >> 31);
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/s_isnan.c
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
