/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): two of the game's small number routines that only its sound
 * code reaches (src/game/sound.c: where a sound is panned to, and whether a wall is
 * between it and the listener), still assembly in the decompilation. Not a PS2 match.
 *
 * Unlike game_fastfunc.c's, these were not written from the routines' instructions:
 * what each does is taken from how the decompilation's matched callers use it
 * (func_0022DB48, func_0022D970) and from ReRAC's port of the same two callers of the
 * US release.
 *
 * Adapted from ReRAC (https://github.com/re-rac/rerac),
 * crates/rc-game/src/audio/voices.rs (pan_degrees, occluded): ISC License, Copyright
 * (c) 2026 ReRAC contributors. NTSC-U 0x22c830 and 0x22c658 are PAL's func_0022DB48
 * and func_0022D970.
 */
#include "common.h"

float openrac_sqrt(float x);

/* x kept between lo and hi: the pan's weight, how far a sound is past one unit from
 * the listener, between 0 and 1. */
float func_001F9BB0(float x, float lo, float hi) {
    if (x < lo) {
        x = lo;
    }
    if (hi < x) {
        x = hi;
    }
    return x;
}

/* out = a, shortened to the length `longest` when it is longer, in x, y and z; w is
 * a's. The occlusion test's line ends at most 64 units from the listener. */
void func_001F9E58(float *out, float *a, float longest) {
    float x = a[0];
    float y = a[1];
    float z = a[2];
    unsigned w = ((unsigned *)a)[3];
    float length = openrac_sqrt(x * x + y * y + z * z);

    if (length != 0.0f && length >= longest) {
        float scale = longest / length;
        x = x * scale;
        y = y * scale;
        z = z * scale;
    }
    out[0] = x;
    out[1] = y;
    out[2] = z;
    ((unsigned *)out)[3] = w;
}
