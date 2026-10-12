#include <stdarg.h>
#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x116070-0x116248. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];

LINKER_REMNANT("asm/remnants/core_text", func_00116070);

extern int func_001170A0(void *, const char *, va_list); /* vfprintf */

/* printf, from the C library (newlib): points _stdout's _data back at
   the reentrancy struct (_impure_ptr, D_0012F86C) and hands the va_list
   to vfprintf. _REENT is read once into a local: retail keeps it in a
   register across the store. Built with 2.9-ee, which saves only the
   integer argument registers (Makefile.sn, EE29_CORE). */
int func_00116078(const char *fmt, ...) {
    int ret;
    va_list ap;
    char *r = (char *)D_0012F86C;

    *(void **)(*(char **)(r + 8) + 0x54) = r;
    va_start(ap, fmt);
    ret = func_001170A0(*(void **)(r + 8), fmt, ap);
    va_end(ap);
    return ret;
}

void func_001160C8(int arg0) {
    *(int *)((char *)D_0012F86C + 0x58) = arg0;
}

/* newlib's rand(), as newlib writes it: one expression that reads
   _REENT (D_0012F86C) twice. +0x58 is _REENT->_next, which srand
   (func_001160C8 above) sets. */
int func_001160D8(void) {
    return (*(unsigned int *)((char *)D_0012F86C + 0x58) =
            *(unsigned int *)((char *)D_0012F86C + 0x58) * 1103515245 + 12345) & 0x7FFFFFFF;
}

extern int func_00119088();

int func_00116108(int *errOut, void *a, void *b, void *c) {
    int r;
    D_0015ED10 = 0;
    r = func_00119088(a, b, c);
    if (r == -1 && D_0015ED10 != 0) {
        *errOut = D_0015ED10;
    }
    return r;
}

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

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/runtime/newlib/double_is_not_nan.c, double_is_not_nan. */
s32 func_00116168(f64 x) {
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
    return 1 - (s32)(((u32)(hx | (-hx))) >> 31);
}


/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/math/classify_float_bits.c, ClassifyDoubleNaN. */
s32 func_001161B0(f64 x) {
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


int func_001161E8(int *errOut, int arg1) {
    unsigned int r;
    D_0015ED10 = 0;
    r = func_00119110(arg1);
    if (r == 0xFFFFFFFF && D_0015ED10 != 0) {
        *errOut = D_0015ED10;
    }
    return r;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116244);
