/* Carried from port/game/rac1-pal/hand/game_int_to_float.c through the version map (../README.md): names and
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

/* Ratchet & Clank (NTSC-U): the integer as a float (mtc1, cvt.s.w). The NTSC decomp keeps the
 * routine as assembly because the retail one has no hazard nop after its mtc1; PAL's
 * decompilation has it as this C (its func_001FA888). */
float func_001FA6C0(int value) {
    return (float)value;
}
