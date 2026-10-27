/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: follow-camera routines whose decompiled C loses what retail returns
 * (hostgen.json / libraries.tsv rows: the port's version decides the prototype callers see).
 */
#include "rac1.h"

#include <string.h>

/* func_L00_001EB6A8 (camera_001EB508.c, matched C declared void): steers *p toward the angle
 * difference FastDiffRots(b, a) with damping c / d, clamps it to +-lim (when lim is not 0) and to
 * +-|difference|, and ends in a tail call of FastAddRots(a, *p), whose result retail returns in $f0.
 * The camera rows (func_L00_002EA4C0) and other callers read that result. */
float func_L00_001EB6A8(gaddr p, float a, float b, float c, float d, float lim) {
    const float r = func_001FA790(b, a);
    float v;
    memcpy(&v, G(p), 4);
    v = v + (c * r - d * v);
    if (lim != 0.0f) {
        if (lim < v) {
            v = lim;
        } else if (v < -lim) {
            v = -lim;
        }
    }
    if (func_001F9B88(r) < v) {
        v = func_001F9B88(r);
    } else if (v < -func_001F9B88(r)) {
        v = -func_001F9B88(r);
    }
    memcpy(G(p), &v, 4);
    return func_001FA748(a, v);
}

#if !RAC1_OMIT(func_001EC210)
/* func_001EC210 (Camera_handleCollWithHero, games/rac1/src/game/camera.c:97). The executable's copy
 * calls the empty stub func_001E97C8; each level's copy calls the level's create_camera_collision_moby
 * (func_L00_002A6070: a camera_object, class 1007, at the eye), which the hero meets when he faces the
 * camera (the runners' camera proxy). The C's call went to a return-0 place of the stub, so the moby
 * was never made. */
void func_001EC210(gaddr cam) {
    const gaddr follow = OPENRAC_DATA(RAC1_ADDR(0x00187040u)) + 0x190u;
    gaddr* moby = &GREF(gaddr, follow + 0xC4u);
    if (GREF(int16_t, cam + 0x86u) == 0) {
        if (*moby == 0 && openrac_guest_overlay() >= 0) {
            *moby = func_L00_002A6070(follow - 0x50u); /* &gCamera.pos */
        }
    } else if (*moby != 0) {
        func_0020D678(*moby); /* DeleteMoby */
        *moby = 0;
    }
}
#endif
