#include "common.h"
#include "structs.h"
#include <stdarg.h>

/*
 * The two functions below are fdlibm's, as newlib carries them:
 *
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 *
 * The do { } while (0) in each is fdlibm's own EXTRACT_WORDS macro, written out.
 */
typedef union {
    f64 value;
    struct {
        u32 lsw;
        u32 msw;
    } parts;
} ieee_double_shape_type;

/* rac1-decomp src/core/00116070.c: func_001161B0 */
/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/math/classify_float_bits.c, ClassifyDoubleNaN. */
s32 func_001313C8(f64 x) {
    s32 hx;
    s32 lx;
    ieee_double_shape_type ew_u;

    do {
        ew_u.value = x;
        hx = ew_u.parts.msw;
        lx = ew_u.parts.lsw;
    } while (0);
    hx &= 0x7fffffff;
    hx |= (u32)(lx | (-lx)) >> 31;
    hx = 0x7ff00000 - hx;
    return (u32)hx >> 31;
}
