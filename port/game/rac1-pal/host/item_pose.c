/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): the three updates that pose an item from Ratchet's pose, as the levels'
 * copies of them run (hostgen.json, "host_functions"; the decompilation's C is src/game/pause.c).
 *
 * HandItemUpdate (func_00225E00), func_00225FB8 and func_002260A8 (the pause menu's previews) call
 * the executable's empty stub func_001E9800 where every level's copy of them calls the level's
 * HeroItemPoseFromRatchet (level 0: func_L00_00209940) with an index list and an output buffer,
 * and point the item moby's bindings (+0x68, +0x6C) at that output. The executable forms all of
 * these, and the class table HandItemUpdate searches, as the one address D_001864D0; a level's copy
 * forms each as an address of its own (level 0: the table 0x179BC0, the lists 0x17A6C0, 0x17A708,
 * 0x15F788, 0x17A750 and 0x17A768, the outputs 0x17C580, 0x17C780, 0x17C980 and 0x17CA80), which
 * one relocation of D_001864D0 cannot give. Here each is read from the loaded level's copy of the
 * function, at the lui and addiu that form it there (the copies are the executable's code with
 * other addresses), and so is the function its call goes to. Without a level (or where the copy's
 * code is not as expected) they are the executable's: the stub, and D_001864D0 relocated.
 */
#include <stdint.h>
#include <stdio.h>

#include "game_protos.h"
#include "openrac/game_host.h"

#define D_001864D0 0x001864D0u

/* The address the loaded level's copy of executable function `fn` forms with the lui at fn + hi
 * and the addiu (or daddiu) at fn + lo that adds to its register, or `fallback`. */
static gaddr copy_address(gaddr fn, uint32_t hi, uint32_t lo, gaddr fallback) {
    const gaddr copy = OPENRAC_CODE(fn);
    if (openrac_guest_overlay() < 0 || copy == fn) {
        return fallback;
    }
    const uint32_t a = GREF(uint32_t, copy + hi), b = GREF(uint32_t, copy + lo);
    if ((a >> 26) != 0x0F || ((b >> 26) != 0x09 && (b >> 26) != 0x19)
        || ((b >> 21) & 31) != ((a >> 16) & 31)) {
        static int told;
        if (!told) {
            told = 1;
            fprintf(
                stderr,
                "[warning] item pose: the copy of %08x at %08x does not form an address at "
                "+%x/+%x\n",
                fn,
                copy,
                hi,
                lo
            );
        }
        return fallback;
    }
    return ((a & 0xFFFFu) << 16) + (gaddr)(int32_t)(int16_t)(b & 0xFFFFu);
}

/* The function the loaded level's copy of `fn` calls with the jal at fn + at, or 0. */
static gaddr copy_call(gaddr fn, uint32_t at) {
    const gaddr copy = OPENRAC_CODE(fn);
    if (openrac_guest_overlay() < 0 || copy == fn) {
        return 0;
    }
    const uint32_t w = GREF(uint32_t, copy + at);
    return (w >> 26) == 3 ? (copy & 0xF0000000u) | ((w & 0x03FFFFFFu) << 2) : 0;
}

/* HeroItemPoseFromRatchet(list, out, model, extra, moby), when the copy calls it. */
static void item_pose(
    gaddr fn, uint32_t at, gaddr list, gaddr out, gaddr model, int extra, int moby
) {
    const gaddr target = copy_call(fn, at);
    if (target != 0) {
        GFN(void (*)(gaddr, gaddr, gaddr, int, int), target)(list, out, model, extra, moby);
    }
}

/* The source moby of an item: its class's +0x44 (moby +0x78 -> word 0). */
static int source_moby(gaddr m) {
    return GREF(int, GREF(gaddr, GREF(gaddr, m + 0x78)) + 0x44);
}

/* HandItemUpdate (the hand items' update): the item at its class record's bone of the source moby;
 * a still item takes its pose from Ratchet's (copy: list +0x138/+0x144, output +0x134/+0x13C, the
 * call at +0x150; the class table +0x60/+0x70). */
void func_00225E00(gaddr m) {
    GFRAME(frame_, 0x40);
    const gaddr fn = 0x00225E00u;
    const int id = source_moby(m);
    if ((GREF(uint8_t, m + 0x70) & 2) != 0 && GREF(uint8_t, m + 0x53) != 1) {
        func_00213DE0(m, 1, 0, 0);
    }
    const gaddr table = copy_address(fn, 0x60, 0x70, OPENRAC_DATA(D_001864D0));
    const int16_t oclass = GREF(int16_t, m + 0xA6);
    int i;
    for (i = 0; i < 0x25; i++) {
        if (GREF(int, table + (gaddr)i * 0x4Cu + 0x10) == oclass) {
            break;
        }
    }
    const gaddr rec = table + (gaddr)i * 0x4Cu;
    func_0020DAF8((gaddr)id, GREF(int, rec + 0xC), frame_);
    openrac_qcopy(m + 0x10, frame_ + 0x30);
    func_0020ED48(m);
    const int still = GREF(int, rec + 0x18) == 0;
    func_001FA480(m + 0xC0, frame_);
    if (!still) {
        func_00214F78(m + 0xC0);
    }
    func_0020EEE8(m);
    int sync = 0;
    const gaddr s = OPENRAC_DATA(0x001D6160u);
    if (GREF(uint8_t, s + 1) != 0) {
        sync = 1;
        func_0020D9D8((gaddr)id, s);
    }
    if (still) {
        const gaddr list = copy_address(fn, 0x138, 0x144, OPENRAC_DATA(D_001864D0));
        const gaddr out = copy_address(fn, 0x134, 0x13C, OPENRAC_DATA(D_001864D0));
        item_pose(fn, 0x150, list, out, GREF(gaddr, m + 0x24), 0, id);
        GREF(gaddr, m + 0x68) = out;
        GREF(int, m + 0x54) = 0;
        GREF(gaddr, m + 0x6C) = out;
        GREF(int, m + 0x58) = 0;
        GREF(int, m + 0x50) = 0;
    }
    if (sync) {
        func_0020D960((gaddr)id, 0, s);
        GREF(int, s + 0x20) = 0;
        GREF(int, s + 0x24) = 0;
        GREF(int, s + 0x28) = 0;
    }
}

/* An item at bone 4 of its source moby, posed from Ratchet's: class 0x1B1 with its own list and 6
 * extra entries (copy: list +0x80/+0x88, output +0x7C/+0x90, the call at +0x94), any other with
 * the default list (+0xA4/+0xAC, output +0x7C/+0xB4, the call at +0xB8). */
void func_00225FB8(gaddr m) {
    GFRAME(frame_, 0x40);
    const gaddr fn = 0x00225FB8u;
    const int id = source_moby(m);
    func_0020E3D0(m);
    func_0020ED48(m);
    func_0020DAF8((gaddr)id, 4, frame_);
    openrac_qcopy(m + 0x10, frame_ + 0x30);
    func_001FA480(m + 0xC0, frame_);
    func_00214F78(m + 0xC0);
    func_0020EEE8(m);
    gaddr out;
    if (GREF(int16_t, m + 0xA6) == 0x1B1) {
        const gaddr list = copy_address(fn, 0x80, 0x88, OPENRAC_DATA(D_001864D0));
        out = copy_address(fn, 0x7C, 0x90, OPENRAC_DATA(D_001864D0));
        item_pose(fn, 0x94, list, out, GREF(gaddr, m + 0x24), 6, id);
    } else {
        const gaddr list = copy_address(fn, 0xA4, 0xAC, OPENRAC_DATA(D_001864D0));
        out = copy_address(fn, 0x7C, 0xB4, OPENRAC_DATA(D_001864D0));
        item_pose(fn, 0xB8, list, out, GREF(gaddr, m + 0x24), 0, id);
    }
    GREF(int, m + 0x50) = 0;
    GREF(gaddr, m + 0x68) = out;
    GREF(int, m + 0x54) = 0;
    GREF(gaddr, m + 0x6C) = out;
}

/* The pause menu's two preview items, at bone 2 (or 3, the second) of the source moby, each posed
 * from Ratchet's with its own list and output (copy: the first list +0xF0/+0x100 and output
 * +0xEC/+0xF4, the second's +0x104/+0x110 and +0xE8/+0x108; one call, at +0x11C), the manipulators
 * 0x17 and 0x16 taken off the source moby meanwhile. */
void func_002260A8(gaddr moby) {
    GFRAME(frame_, 0x40);
    const gaddr fn = 0x002260A8u;
    const gaddr vars = GREF(gaddr, moby + 0x78);
    const int id = GREF(int, GREF(gaddr, vars) + 0x44);
    func_0020E3D0(moby);
    func_0020ED48(moby);
    const int second = moby == GREF(gaddr, GREF(gaddr, vars) + 0x5C);
    func_0020DAF8((gaddr)id, second ? 3 : 2, frame_);
    openrac_qcopy(moby + 0x10, frame_ + 0x30);
    func_001FA480(moby + 0xC0, frame_);
    func_00214F78(moby + 0xC0);
    func_0020EEE8(moby);
    const gaddr first_m = OPENRAC_DATA(0x001D61E0u), second_m = OPENRAC_DATA(0x001D61A0u);
    int first_on = 0, second_on = 0;
    if (GREF(uint8_t, first_m + 1) != 0) {
        first_on = 1;
        func_0020D9D8((gaddr)id, first_m);
    }
    if (GREF(uint8_t, second_m + 1) != 0) {
        second_on = 1;
        func_0020D9D8((gaddr)id, second_m);
    }
    const gaddr list = second ? copy_address(fn, 0x104, 0x110, OPENRAC_DATA(D_001864D0))
                              : copy_address(fn, 0xF0, 0x100, OPENRAC_DATA(D_001864D0));
    const gaddr out = second ? copy_address(fn, 0xE8, 0x108, OPENRAC_DATA(D_001864D0))
                             : copy_address(fn, 0xEC, 0xF4, OPENRAC_DATA(D_001864D0));
    item_pose(fn, 0x11C, list, out, GREF(gaddr, moby + 0x24), 0, id);
    GREF(gaddr, moby + 0x68) = out;
    GREF(gaddr, moby + 0x6C) = out;
    if (first_on) {
        func_0020D960((gaddr)id, 0x17, first_m);
        GREF(int, first_m + 0x20) = 0;
        GREF(int, first_m + 0x24) = 0;
        GREF(int, first_m + 0x28) = 0;
    }
    if (second_on) {
        func_0020D960((gaddr)id, 0x16, second_m);
        GREF(int, second_m + 0x20) = 0;
        GREF(int, second_m + 0x24) = 0;
        GREF(int, second_m + 0x28) = 0;
    }
    GREF(int, moby + 0x54) = 0;
    GREF(int, moby + 0x50) = 0;
}
