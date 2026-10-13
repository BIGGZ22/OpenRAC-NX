/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): functions the decompilation's catalogue (config/overlays/functions.tsv)
 * folds into another by its fingerprint, although their code differs from it in a constant or a
 * field offset (split_places.tsv). Each is the function it was folded into with the words that
 * differ, read from the retail copies; the places are theirs alone (hostgen, read_split_places).
 */
/* Carried from port/game/rac1-pal/host/variants.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <stdint.h>

/* The hero block, D_0013E533 + 0xE1D. */
#define HERO 0x0013F350u

/* game_FUN_001ebec8's level copy: the dispatch record's second hook (+0x10), not its first (+0x08). */
void func_L00_001EB9D8(gaddr arg0) {
    const gaddr rec = OPENRAC_DATA(0x001E8C00u) + (gaddr)GREF(short, arg0 + 0x8C) * 0x14u;
    const gaddr fn = GREF(gaddr, rec + 0x10);
    if (fn != 0) {
        GFN(void (*)(gaddr), fn)(arg0);
    }
}

/* game_FUN_001fb680's level copies: the same four-word packet (a DMA call of COUNT quadwords at DATA
 * and its return) with other counts and data. */
static void packet_call(uint32_t count, gaddr data) {
    const gaddr at = GREF(gaddr, OPENRAC_DATA(0x00160F00u));
    GREF(uint32_t, at + 0) = 0x30000000u | count;
    GREF(uint32_t, at + 4) = data;
    GREF(uint32_t, at + 8) = 0;
    GREF(uint32_t, at + 12) = 0x50000000u | count;
    GREF(gaddr, OPENRAC_DATA(0x00160F00u)) = at + 16;
}

void func_L00_00201350(void) { packet_call(0x29, 0x00151B60u); }
/* Three neighbours in each level, each calling its own three quadwords: level 0's data addresses,
 * relocated to the loaded level's by pairing each with its copy there. */
void func_L00_002A0D90(void) { packet_call(0x03, OPENRAC_LDATA(0, 0x001C2F00u)); }
void func_L00_002A0DF0(void) { packet_call(0x03, OPENRAC_LDATA(0, 0x001C24C0u)); }
void func_L00_002A0E50(void) { packet_call(0x03, OPENRAC_LDATA(0, 0x001C24F0u)); }
void func_L00_002A0EB8(void) { packet_call(0x0B, 0x0013CF10u); }

/* game_FUN_0020be38's level copy: flags +0x20 and +0x21 of D_0013D4C0, not +0x21 and +0x1F. */
int func_L00_0024E01C(void) {
    const gaddr base = 0x0013D4C0u;
    return GREF(uint8_t, base + 0x20) != 0 && GREF(uint8_t, base + 0x21) != 0;
}

/* game_FUN_002169c0's level copy: the track words at +0x38/+0x3C/+0x3A of D_001516D0, not
 * +0x54/+0x58/+0x56. */
void func_L00_00266238(int arg0, long long arg1) {
    const gaddr p = (gaddr)(int)arg1;
    if (p == 0) {
        return;
    }
    GREF(int, p) = arg0;
    if (arg0 != 0) {
        if (GREF(short, p + 10) == 1) {
            GREF(short, p + 10) = 2;
        }
    } else {
        const gaddr b = 0x001516D0u;
        game_FUN_00215970(GREF(short, b + 0x38), GREF(short, b + 0x3C), GREF(short, b + 0x3A));
    }
}

/* game_FUN_002288f0's level copy: words 6 and 5 of D_0018A2B0 decide the shrub textures, not 8 and 7. */
void func_L00_002959F8(void) {
    const gaddr out = OPENRAC_DATA(0x00160F00u);
    const gaddr list = OPENRAC_DATA(0x001603F0u);
    const gaddr p = GREF(gaddr, out);
    GREF(gaddr, out) += 16;
    const gaddr l = GREF(gaddr, list);
    GREF(int, l + 0) = 0x20000000;
    GREF(int, l + 4) = (int)GREF(gaddr, out);
    GREF(int, l + 8) = 0;
    GREF(int, l + 12) = 0;
    const gaddr flags = OPENRAC_DATA(0x0018A2B0u);
    if (GREF(int, flags + 6 * 4) != 0 && GREF(int, flags + 5 * 4) != 0) {
        const int size = game_FUN_0022a330(GREF(int, 0x0015EE74u));
        game_FUN_00233b68();
        if (GREF(int, OPENRAC_DATA(0x001603F8u)) < size) {
            GREF(int, OPENRAC_DATA(0x001603F8u)) = size;
        }
    }
    const gaddr q = GREF(gaddr, out);
    GREF(int, q + 0) = 0x20000000;
    GREF(int, q + 4) = (int)(GREF(gaddr, list) + 16);
    GREF(int, q + 8) = 0;
    GREF(int, q + 12) = 0;
    GREF(gaddr, out) += 16;
    GREF(int, p + 0) = 0x20000000;
    GREF(int, p + 4) = (int)GREF(gaddr, out);
    GREF(int, p + 8) = 0;
    GREF(int, p + 12) = 0;
}

/* game_FUN_L05_00319598 (levels 5 and 16; no C in the decompilation): the moby of the class list
 * D_L05_001ABCC0[moby +0x21] (moby indices, the last with its top bit set) whose variables
 * (+0x78) hold ID at +0xB4, or 0. Level 16's copy compares +0xAC. */
static gaddr moby_with_id(gaddr moby, int id, uint32_t field) {
    const gaddr list = GREF(gaddr, OPENRAC_LDATA(5, 0x001ABCC0u) + 4u * GREF(uint8_t, moby + 0x21));
    if (list == 0) {
        return 0;
    }
    const gaddr base = GREF(gaddr, OPENRAC_LDATA(5, 0x0015FFD8u));
    for (gaddr p = list;; p += 2) {
        const uint16_t w = GREF(uint16_t, p);
        const gaddr m = base + (gaddr)(w & 0x7FFF) * 0x100u;
        if (GREF(int16_t, GREF(gaddr, m + 0x78) + field) == id) {
            return m;
        }
        if (w & 0x8000) {
            return 0;
        }
    }
}

/* game_FUN_002141f8's two neighbours. All three give the block a moby with mode bit 0x20 keeps a
 * pointer to in its variables, or 0 for no moby or one without the bit; game_FUN_002141f8 reads the
 * pointer at +0 (the block a hit is resolved in), these two the ones at +0x10 and +8. The one at +8
 * is the platform block: the turn and the move a moby made this tick, which it carries whatever
 * stands on it with (game_FUN_L00_00260738 writes it, the hero's game_FUN_L00_00233f80 reads it). Called
 * as game_FUN_002141f8, the carry read a crate's hit block instead, whose first words are its
 * health 1.0 and a 0.5: a turn of one radian and a move of half a unit a tick, which threw the hero
 * off any crate he landed on. */
static int moby_block(gaddr moby, gaddr at) {
    if (moby == 0 || (GREF(uint16_t, moby + 0x34) & 0x20) == 0) {
        return 0;
    }
    return GREF(int, GREF(gaddr, moby + 0x78) + at);
}

int func_L00_0025C368(gaddr moby) { return moby_block(moby, 0x10); }

