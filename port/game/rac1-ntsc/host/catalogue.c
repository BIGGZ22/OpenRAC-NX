/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank (NTSC-U): functions the NTSC decomp's catalogue (config/overlays/us/functions.tsv)
 * lists apart where games/rac1's folds them into one, so the shared host C (port/game/rac1/host)
 * answers only the one games/rac1 names. Each answers as the function it is a copy of. Written in
 * this version's own names.
 */
#include "game_protos.h"
#include "openrac/game_host.h"

/* The levels' own copy of WadDecompress (level 0's 0x24D8D0; the executable's, FUN_0020b618, is answered in
 * port/game/rac1/host/hand.c): the same answer. */
int game_FUN_L00_0024d8d0(int src, int dst) {
    return game_FUN_0020b618(src, dst);
}
