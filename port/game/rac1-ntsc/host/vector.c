/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): vector routines of the game's VU0 library that the shared replacements
 * (game/common/lib/vu0.c) do not have yet, written from what each retail routine does
 * (hostgen.json, "host_functions").
 */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <float.h>
#include <math.h>
#include <string.h>

/* sceVu0Normalize (an entry inside the library object; PAL func_001252C0): out.xyz = v.xyz
 * scaled by 1 / sqrt((x*x + y*y) + z*z), out.w = 0. A zero vector gives the VU's division by zero
 * (the largest float), times zero. */
void game_FUN_00125180(gaddr out, gaddr v) {
    float in[3];
    memcpy(in, G(v), sizeof in);
    const float len = sqrtf((in[0] * in[0] + in[1] * in[1]) + in[2] * in[2]);
    const float q = len == 0.0f ? FLT_MAX : 1.0f / len;
    const float r[4] = {in[0] * q, in[1] * q, in[2] * q, 0.0f};
    memcpy(G(out), r, sizeof r);
}
