/* Carried from port/game/rac1-pal/hand/overlays_shared_pad_00268760.c through the version map (../README.md): names and
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
 * Ratchet & Clank (PAL): the particle step of the level programs, in C.
 *
 * Hand-written VU0 macro code in the retail program, so the decompilation keeps it as assembly.
 * Written from the instructions of the routines named, like the others in this directory.
 *
 * A particle record is 0x40 bytes: +0x04 its colour (four bytes), +0x08 its age, +0x0A the ticks
 * it has left, +0x0C a float that grows by +0x3C each tick, +0x10 its position, +0x20 its
 * velocity (w: what is added to the velocity's z each tick), +0x30 and +0x34 the colours it starts
 * and ends with, +0x38 its lifetime in ticks.
 */
#include "types.h"

extern void func_L00_00267A08(unsigned char *rec);

/* Frees the particle (func_L00_00267A08). */
void func_L00_00267BE8(unsigned char *rec) {
    func_L00_00267A08(rec);
}

/*
 * One tick of a particle: one tick less to live (freed when none are left), the position moved by
 * the velocity, gravity added to the velocity, the age and +0x0C advanced, and the colour blended
 * from the start colour to the end colour by the share of its life gone; the last eight ticks
 * fade its alpha out.
 */
void FUN_L00_00267b10(unsigned char *rec) {
    int left = *(short *)(rec + 0x0A) - 1;
    int life = *(short *)(rec + 0x38);
    unsigned int first = *(unsigned int *)(rec + 0x30);
    unsigned int last = *(unsigned int *)(rec + 0x34);
    float *pos = (float *)(rec + 0x10);
    float *vel = (float *)(rec + 0x20);
    float q;
    float fade;
    float a[4];
    float b[4];
    unsigned int colour = 0;
    int i;

    if (left <= 0) {
        func_L00_00267BE8(rec);
        return;
    }
    *(short *)(rec + 0x0A) = (short)left;
    q = (float)left / (float)life;
    fade = (float)((left < 8 ? left : 8) << 9) / 4096.0f;
    for (i = 0; i < 4; i++) {
        a[i] = (float)((first >> (i * 8)) & 0xFF);
        b[i] = (float)((last >> (i * 8)) & 0xFF);
    }
    pos[0] += vel[0];
    pos[1] += vel[1];
    pos[2] += vel[2];
    vel[2] += vel[3];
    rec[8]++;
    a[3] *= fade;
    b[3] *= fade;
    for (i = 0; i < 4; i++) {
        colour |= ((unsigned int)(int)(b[i] * (1.0f - q) + a[i] * q) & 0xFF) << (i * 8);
    }
    *(float *)(rec + 0x0C) += *(float *)(rec + 0x3C);
    *(unsigned int *)(rec + 0x04) = colour;
}
