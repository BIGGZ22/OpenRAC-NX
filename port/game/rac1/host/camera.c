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
