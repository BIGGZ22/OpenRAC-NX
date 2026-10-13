/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): the moby animation routines that are hand-written VU0 code (hostgen.json,
 * "host_functions"). The game's joint evaluator (game_FUN_002109b8) is not run by the port, whose
 * renderer poses each moby itself from the animation fields (frame, key pair, blend timer); the
 * blend snapshots it reads are written here by the port's own encoder.
 */
/* Carried from port/game/rac1-pal/host/anim.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

/* The blend slots the snapshots go to: 0x800 bytes each from D_001AABC0, the address
 * game_FUN_0020c880 then gives key A (seq 0xFF, frame = slot); each program has its copy. */
#define BLEND_SLOTS 0x001AABC0u

/* game_FUN_0020ede8 (hand-written, 3240 bytes): snapshots a moby's current pose into the blend slot the
 * caller took (game_FUN_00212f90 / game_FUN_002130d8 on a sequence change: a1 = slot | flags), so that the
 * blend starts from it. Done by the port's encoder (openrac_game_moby_snapshot, after ReRAC's); the
 * flags 0x100 / 0x200 skip the pose layers, which the port's snapshot never has. */
void game_FUN_0020ede8(gaddr a0, int a1) {
    openrac_game_moby_snapshot(a0, OPENRAC_DATA(BLEND_SLOTS) + (gaddr)(a1 & 0xFF) * 0x800u);
}

/* func_L00_00251CE8 (hand-written, 2216 bytes): the level programs' variant of game_FUN_0020ede8 for
 * Ratchet (from game_FUN_L00_002323b8): the same pose snapshot into a blend slot. */
void func_L00_00251CE8(gaddr a0, int a1) {
    openrac_game_moby_snapshot(a0, OPENRAC_DATA(BLEND_SLOTS) + (gaddr)(a1 & 0xFF) * 0x800u);
}
