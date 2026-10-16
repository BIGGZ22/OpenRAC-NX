/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: the moby animation routines that are hand-written VU0 code (hostgen.json,
 * "host_functions"). The game's joint evaluator (func_00211808) is not run by the port, whose
 * renderer poses each moby itself from the animation fields (frame, key pair, blend timer); the
 * blend snapshots it reads are written here by the port's own encoder.
 */
#include "rac1.h"

/* The blend slots the snapshots go to: 0x800 bytes each from D_001AAF40, the address
 * func_0020D6D0 then gives key A (seq 0xFF, frame = slot); each program has its copy. */
#define BLEND_SLOTS RAC1_ADDR(0x001AAF40u)

/* func_0020FC38 (hand-written, 3240 bytes): snapshots a moby's current pose into the blend slot the
 * caller took (func_00213DE0 / func_00213F28 on a sequence change: a1 = slot | flags), so that the
 * blend starts from it. Done by the port's encoder (openrac_game_moby_snapshot, after ReRAC's); the
 * flags 0x100 / 0x200 skip the pose layers, which the port's snapshot never has. */
void func_0020FC38(gaddr a0, int a1) {
    openrac_game_moby_snapshot(a0, OPENRAC_DATA(BLEND_SLOTS) + (gaddr)(a1 & 0xFF) * 0x800u);
}

/* func_L00_00252D20 (hand-written, 2216 bytes): the level programs' variant of func_0020FC38 for
 * Ratchet (from func_L00_00232C10): the same pose snapshot into a blend slot. */
void func_L00_00252D20(gaddr a0, int a1) {
    openrac_game_moby_snapshot(a0, OPENRAC_DATA(BLEND_SLOTS) + (gaddr)(a1 & 0xFF) * 0x800u);
}
