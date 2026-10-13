/* Carried from port/game/rac1-pal/hand/overlays_shared_mobyfunc_0024FD50.c through the version map (../README.md): names and
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
 * Ratchet & Clank (PAL): small hand-written assembly routines of the level programs, in C.
 *
 * Like the vector routines in vector.c, these are not compiler output in the retail program
 * (trapping adds, $at as a working register, FPU conversions that never leave the FPU), so the
 * decompilation keeps them as assembly. Each function here was written from the instructions of
 * the routine it is named after and does what those do to memory and to the result register.
 * Only routines that take their arguments and give their result as compiled code does are here:
 * some of the game's assembly passes values in other registers, which C cannot say.
 */
#include "types.h"

/*
 * Packs three values above the low 32 bits of the 64-bit word at 0x38 of a record: a from bit
 * 32, b from bit 40, c from bit 48, each shifted as a whole register so that a longer value runs
 * into the next field. The new word is also the result.
 */
unsigned long FUN_L00_002502f0(char *record, unsigned long a, unsigned long b, unsigned long c) {
    unsigned long *word = (unsigned long *)(record + 0x38);
    unsigned long value = *word & 0xFFFFFFFFul;
    value |= a << 32;
    value |= b << 40;
    value |= c << 48;
    *word = value;
    return value;
}
