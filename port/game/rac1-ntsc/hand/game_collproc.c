/* Carried from port/game/rac1-pal/hand/game_collproc.c through the version map (../README.md): names and
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
 * Ratchet & Clank (PAL): hand-written assembly of src/game/collproc.c, in C, written from what
 * each retail routine does.
 */
#include "types.h"

extern unsigned char D_00194100[];

/* The low five bits of the word at 0x1C of D_00194100, or -1 when the word is negative or those
 * bits are all set. */
int FUN_001f0b58(void) {
    int v = *(int *)(D_00194100 + 0x1C);
    if (v < 0) {
        return -1;
    }
    v &= 0x1F;
    if (v == 0x1F) {
        return -1;
    }
    return v;
}
