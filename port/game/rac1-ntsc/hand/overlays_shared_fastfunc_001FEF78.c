/* Carried from port/game/rac1-pal/hand/overlays_shared_fastfunc_001FEF78.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): the game's small vector and number routines, in C.
 *
 * In the retail program these are hand-written assembly for the EE's vector
 * unit (VU0 macro instructions) and FPU, so the decompilation keeps them as
 * assembly: no C compiles to them. The port needs them in C all the same, and
 * they are the most called code in the game. Each function here was written
 * from the instructions of the routine it is named after and does what those
 * do to memory and to the result register, operation for operation and in the
 * same order, so that rounding agrees; the build's check runs both and
 * compares (runtime/port/README.md).
 *
 * What they do not reproduce is what the routines leave in the vector unit's
 * own registers. A routine whose result depends on what an earlier one left
 * there (the cross product stores a fourth field it never computed) is not
 * here.
 *
 * A vector is four floats, x first. The assembly loads all 128 bits of its
 * operands before it stores, so every function reads all it needs first: the
 * result may be one of the operands.
 */
#include "types.h"

/* The console's square root, and `a` over the square root of `b` as its one operation. */
float openrac_sqrt(float x);
float openrac_rsqrt(float a, float b);

/* How many game ticks one sixtieth of a second is (1.0 at 60 Hz, 1.2 at 50 Hz), read through $gp. */
extern float D_0015ED60;

/* The bits of a float, for a field that is copied and not computed. */
static __inline__ unsigned bits_of(float *v, int field) {
    return ((unsigned *)v)[field];
}

/* Stores three computed fields and the copied fourth. */
static __inline__ void put(float *out, float x, float y, float z, unsigned w) {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    ((unsigned *)out)[3] = w;
}

/* The level programs' own copy of the routine above. */
void func_L00_001FF500(float *out, float *a, float length) {
    func_001F9BF8(out, a, length);
}

/* a = a + b in x, y and z; w stays a's. The first argument is not used. (vadd.xyz, stored back to a) */
void FUN_L00_001ff290(void *unused, float *a, float *b) {
    float x = a[0] + b[0];
    float y = a[1] + b[1];
    float z = a[2] + b[2];
    (void)unused;
    a[0] = x;
    a[1] = y;
    a[2] = z;
}

/* a less the whole multiples of b in it: the quotient's fraction times b. (vdiv, vftoi0, vitof0) */
float FUN_L00_00200260(float a, float b) {
    float quotient = 0.0f + a / b;
    float whole = (float)(int)quotient;
    return (quotient - whole) * b;
}

/*
 * out = a scaled to the length `length` over x and y; z and w are a's. A vector whose squared
 * length over x and y is exactly zero gets zeros there.
 */
void func_L00_001FF550(float *out, float *a, float length) {
    float x = a[0] * a[0];
    float y = a[1] * a[1];
    float sum = x + y;
    unsigned z = bits_of(a, 2);
    unsigned w = bits_of(a, 3);
    float scale;
    if (bits_of(&sum, 0) == 0) {
        x = 0.0f + 0.0f;
        y = 0.0f + 0.0f;
    } else {
        scale = openrac_rsqrt(length, sum);
        x = a[0] * scale;
        y = a[1] * scale;
    }
    out[0] = x;
    out[1] = y;
    ((unsigned *)out)[2] = z;
    ((unsigned *)out)[3] = w;
}

/*
 * Counts a byte down by one: 1 if it was zero already (nothing is stored), 0 if it is still
 * above zero after the step, 2 if the step brought it to zero.
 */
int FUN_L00_001fefc8(unsigned char *counter) {
    int value = *counter;
    if (value == 0) {
        return 1;
    }
    value = value - 1;
    *counter = (unsigned char)value;
    return value > 0 ? 0 : 2;
}

/* Splits x: the whole part, cut towards zero, goes to *whole and the rest is returned. */
float FUN_L00_00200228(float *whole, float x) {
    float cut = (float)(int)x;
    *whole = cut;
    return x - cut;
}
