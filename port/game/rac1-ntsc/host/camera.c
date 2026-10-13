/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): follow-camera routines whose decompiled C loses what retail returns
 * (hostgen.json / libraries.tsv rows: the port's version decides the prototype callers see).
 */
/* Carried from port/game/rac1-pal/host/camera.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <string.h>

/* game_FUN_L00_001eb328 (camera_001EB508.c, matched C declared void): steers *p toward the angle
 * difference FastDiffRots(b, a) with damping c / d, clamps it to +-lim (when lim is not 0) and to
 * +-|difference|, and ends in a tail call of FastAddRots(a, *p), whose result retail returns in $f0.
 * The camera rows (func_L00_002E9010) and other callers read that result. */
float game_FUN_L00_001eb328(gaddr p, float a, float b, float c, float d, float lim) {
    const float r = game_FUN_001fa5c8(b, a);
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
    if (func_001F99C0(r) < v) {
        v = func_001F99C0(r);
    } else if (v < -func_001F99C0(r)) {
        v = -func_001F99C0(r);
    }
    memcpy(G(p), &v, 4);
    return game_FUN_001fa580(a, v);
}
