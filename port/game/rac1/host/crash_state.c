/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: what a crash report (port/common/crash.h) says of the game beyond the level and
 * the hero: the moby whose update was running, and where the camera was.
 *
 * The moby. The game updates its mobys in one loop (func_00213C78, UpdateMobys; the executable's
 * and each level's copy of it are the one host function), run once a tick by the level's main
 * update (func_L00_00299250 and its kind) and by the cutscene and pause updates. Its C is the
 * decompilation's (games/rac1 nonmatching/game/func_00213C78.c; the US disc's FUN_00212e28), run
 * here step by step with the moby noted before its turn and forgotten after it: one store each way
 * per moby per tick. The decompilation's own copy stays as func_00213C78__game, unused; the two
 * are the same loop, which the files' comments describe word for word.
 *
 * A report asks for the moby while it is written, perhaps in a signal handler: the fields are read
 * from game memory then (oclass at +0xA6, the position at +0x10, the placement id at +0xB2: moby.h)
 * and only once the address is known to lie in main memory, which is always there.
 *
 * The camera: the camera block's position (D_00187040 + 0x140, gCamera; the level's copy while a
 * level is loaded: OPENRAC_DATA), sampled once a frame with the hero.
 */
#include "rac1.h"

#include <stdint.h>

/* The moby list's head as the loop keeps it (D_00160024). */
#define MOBY_HEAD RAC1_ADDR(0x00160024u)
/* The camera block (camera.h, CameraSystem; viewer/game_state.cpp: +0x140 its position). */
#define CAMERA RAC1_ADDR(0x00187040u)
#define CAMERA_POSITION 0x140u

enum {
    MOBY_POS = 0x10,    /* Vec4f pos */
    MOBY_STATE = 0x20,  /* u8 state: negative as a signed byte, not updated */
    MOBY_NEXT = 0x28,   /* struct Moby *next */
    MOBY_FLAGS = 0x34,  /* u16 flags: 0x40 no pre-update step, 0x4 no post-update step */
    MOBY_UPDATE = 0x74, /* void (*update)(struct Moby *) */
    MOBY_OCLASS = 0xA6, /* s16 oclass */
    MOBY_UID = 0xB2,    /* u16 save_id: its place in the level's placements; 0xFFFF none */
    MOBY_SIZE = 0x100
};

/* The moby whose turn of the loop is running, or 0. Volatile: a signal handler reads it. */
static volatile gaddr g_updating;

/* UpdateMobys: every moby of the list whose state is not negative, its pre-update step (unless
 * flag 0x40), its own update when it has one, its post-update step (unless flag 0x4). */
void func_00213C78(void) {
    const gaddr outer = g_updating; /* an update that ran the loop itself (none known) */
    gaddr m = func_0020E6B8();
    GREF(gaddr, OPENRAC_DATA(MOBY_HEAD)) = m;
    while (m != 0) {
        if (GREF(signed char, m + MOBY_STATE) >= 0) {
            g_updating = m;
            if (!(GREF(unsigned short, m + MOBY_FLAGS) & 0x40)) {
                func_0020E3D0(m);
            }
            const gaddr update = GREF(gaddr, m + MOBY_UPDATE);
            if (update != 0) {
                GFN(void (*)(gaddr), update)(m);
            }
            if (!(GREF(unsigned short, m + MOBY_FLAGS) & 0x4)) {
                func_0020ED48(m);
            }
            g_updating = outer;
        }
        m = GREF(gaddr, m + MOBY_NEXT);
    }
}

static int updating_moby(openrac_game_moby *out) {
    const gaddr m = g_updating;
    if (m == 0 || m > GUEST_RAM + GUEST_RAM_SIZE - MOBY_SIZE) {
        return 0;
    }
    out->address = m;
    out->oclass = GREF(int16_t, m + MOBY_OCLASS);
    for (int i = 0; i < 3; ++i) {
        out->position[i] = GREF(float, m + MOBY_POS + 4u * (unsigned)i);
    }
    const uint16_t uid = GREF(uint16_t, m + MOBY_UID);
    out->uid = uid == 0xFFFFu ? -1 : (int32_t)uid;
    return 1;
}

static int camera_position(float xyz[3]) {
    const int level = openrac_game_loaded_overlay();
    if (level == OPENRAC_OVERLAY_EXE) {
        return 0;
    }
    const gaddr at = OPENRAC_DATA(CAMERA) + CAMERA_POSITION;
    for (int i = 0; i < 3; ++i) {
        xyz[i] = GREF(float, at + 4u * (unsigned)i);
    }
    return 1;
}

__attribute__((constructor)) static void install_crash_state(void) {
    openrac_game_updating_moby = updating_moby;
    openrac_game_camera_position = camera_position;
}
