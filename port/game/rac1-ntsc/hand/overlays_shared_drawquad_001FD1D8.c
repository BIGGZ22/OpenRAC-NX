/* Carried from port/game/rac1-pal/hand/overlays_shared_drawquad_001FD1D8.c through the version map (../README.md): names and
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
 * Ratchet & Clank (PAL): a hand-written packet builder of the level programs, in C.
 *
 * Written from the instructions of the routine named, like the others in this directory.
 */
#include "types.h"

/* Where the next VIF packet goes (read and written through $gp). */
extern unsigned char *D_00161180;
/* The strip's packet head (five quadwords) and the unpack headers for its three arrays. */
extern unsigned char D_L00_001C4020[];
extern unsigned char D_L00_00160EC0[];
extern unsigned char D_L00_00160EE0[];
extern unsigned char D_L00_00160F00[];

static unsigned char *copy_quad(unsigned char *out, const unsigned char *from, unsigned int or_last) {
    int i;
    for (i = 0; i < 16; i++) {
        out[i] = from[i];
    }
    *(unsigned int *)(out + 12) |= or_last;
    return out + 16;
}

static unsigned char *pad_to_quad(unsigned char *out) {
    while (((unsigned int)(unsigned long)out & 0xF) != 0) {
        *(unsigned int *)out = 0;
        out += 4;
    }
    return out;
}

/*
 * Adds a strip of n vertices to the VIF packet: the head (its second quadword's first word gets
 * n), then the n texture coordinates (8 bytes each), colours (4 bytes) and positions (12 bytes),
 * each behind its unpack header with n in the header's top half and padded to a quadword, then
 * a call of the VU1 program (entry 12, or 14 with flag bit 0) and a flush. The head's first word
 * gets the packet's length in quadwords less one.
 */
void FUN_L00_001fde98(int n, unsigned char *xyz, unsigned char *colours, unsigned char *uvs, int flag) {
    unsigned char *start = D_00161180;
    unsigned char *out = start;
    unsigned int count = (unsigned int)n << 16;
    int i;

    for (i = 0; i < 5; i++) {
        out = copy_quad(out, D_L00_001C4020 + i * 16, 0);
    }
    *(unsigned int *)(start + 0x10) |= (unsigned int)n;

    out = copy_quad(out, D_L00_00160EC0, count);
    for (i = 0; i < n; i++) {
        *(unsigned int *)out = *(unsigned int *)(uvs + i * 8);
        *(unsigned int *)(out + 4) = *(unsigned int *)(uvs + i * 8 + 4);
        out += 8;
    }
    out = pad_to_quad(out);

    out = copy_quad(out, D_L00_00160EE0, count);
    for (i = 0; i < n; i++) {
        *(unsigned int *)out = *(unsigned int *)(colours + i * 4);
        out += 4;
    }
    out = pad_to_quad(out);

    out = copy_quad(out, D_L00_00160F00, count);
    for (i = 0; i < n; i++) {
        *(unsigned int *)out = *(unsigned int *)(xyz + i * 12);
        *(unsigned int *)(out + 4) = *(unsigned int *)(xyz + i * 12 + 4);
        *(unsigned int *)(out + 8) = *(unsigned int *)(xyz + i * 12 + 8);
        out += 12;
    }
    out = pad_to_quad(out);

    *(unsigned int *)out = 0x15000000u | (unsigned int)(((flag & 1) << 1) + 12);
    *(unsigned int *)(out + 4) = 0x11000000u;
    *(unsigned int *)(out + 8) = 0;
    *(unsigned int *)(out + 12) = 0;
    out += 16;

    *(unsigned int *)start |= (unsigned int)((out - start) >> 4) - 1;
    D_00161180 = out;
}
