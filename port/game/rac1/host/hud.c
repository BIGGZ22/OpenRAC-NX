/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: the level HUD in a picture wider than the game's.
 *
 * The HUD is 13 slots of 0x90 bytes (D_00199C60), each an element with a place on the game's
 * 512-pixel screen (+0x50 x, +0x54 y), anchor flags (+0x60: bit 0 top, bit 1 bottom, bit 2 left,
 * bit 3 right; neither of a pair is centred) and a draw callback (+0x18), which func_001FFFB8 calls
 * for each slot once a frame. Every element draws from its +0x50: the bolt counter and the
 * nanotech straight from it, the meters through func_L00_00236400, which turns it into a corner by
 * the flags.
 *
 * With the player's HUD setting on "Edges" (openrac_game_hud_side), the elements anchored to the
 * screen's left or right are drawn further out by the width the picture gained on that side, so
 * the weapon, the bolts and the meters sit at the picture's sides as they sat at the television's;
 * centred ones stay in the middle. The place is moved only around the game's own draw and put back
 * after it (by the same amount, so whatever the draw itself did to it stays): nothing else in the
 * game sees another HUD.
 *
 * The idea is ReRAC's, which moves the same anchors out by the same flags (crates/rc-game/src/hud.rs,
 * `anchor`, and crates/rc-engine/src/display.rs, `side_extra`; ISC License, Copyright (c) 2026 ReRAC
 * contributors); no code of it is used.
 */
#include "rac1.h"

enum { SLOTS = 13, SLOT_BYTES = 0x90, SLOT_X = 0x50, SLOT_FLAGS = 0x60, ANCHOR_LEFT = 4, ANCHOR_RIGHT = 8 };

void func_001FFFB8(void) {
    const int side = openrac_game_hud_side();
    const gaddr slots = OPENRAC_DATA(RAC1_ADDR(0x00199C60u));
    int moved[SLOTS];
    for (int i = 0; i < SLOTS; i++) {
        const gaddr slot = slots + (gaddr)i * SLOT_BYTES;
        const int flags = GREF(int, slot + SLOT_FLAGS);
        moved[i] = side == 0 ? 0 : (flags & ANCHOR_LEFT) != 0 ? -side : (flags & ANCHOR_RIGHT) != 0 ? side : 0;
        GREF(int, slot + SLOT_X) += moved[i];
    }
    func_001FFFB8__game();
    for (int i = 0; i < SLOTS; i++) {
        GREF(int, slots + (gaddr)i * SLOT_BYTES + SLOT_X) -= moved[i];
    }
}
