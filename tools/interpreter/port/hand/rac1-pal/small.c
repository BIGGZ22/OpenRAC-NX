/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): small hand-written assembly routines of the level programs, in C.
 *
 * Like the vector routines in vector.c, these are not compiler output in the retail program
 * (trapping adds, $at as a working register, FPU conversions that never leave the FPU), so the
 * decompilation keeps them as assembly. Each function here was written from the instructions of
 * the routine it is named after and does what those do to memory and to the result register.
 * Only routines that take their arguments and give their result as compiled code does are here:
 * some of the game's assembly passes values in other registers, which C cannot say.
 */
#include "common.h"

/*
 * Counts a byte down by one: 1 if it was zero already (nothing is stored), 0 if it is still
 * above zero after the step, 2 if the step brought it to zero.
 */
int func_L00_001FEF78(unsigned char *counter) {
    int value = *counter;
    if (value == 0) {
        return 1;
    }
    value = value - 1;
    *counter = (unsigned char)value;
    return value > 0 ? 0 : 2;
}

/* Splits x: the whole part, cut towards zero, goes to *whole and the rest is returned. */
float func_L00_002001D8(float *whole, float x) {
    float cut = (float)(int)x;
    *whole = cut;
    return x - cut;
}

/*
 * Packs three values above the low 32 bits of the 64-bit word at 0x38 of a record: a from bit
 * 32, b from bit 40, c from bit 48, each shifted as a whole register so that a longer value runs
 * into the next field. The new word is also the result.
 */
unsigned long func_L00_00251328(char *record, unsigned long a, unsigned long b, unsigned long c) {
    unsigned long *word = (unsigned long *)(record + 0x38);
    unsigned long value = *word & 0xFFFFFFFFul;
    value |= a << 32;
    value |= b << 40;
    value |= c << 48;
    *word = value;
    return value;
}
