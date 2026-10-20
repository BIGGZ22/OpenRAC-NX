/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: vector routines of the game's VU0 library that the shared replacements
 * (game/common/lib/vu0.c) do not have yet, written from what each retail routine does
 * (hostgen.json, "host_functions").
 */
#include "rac1.h"

#include <float.h>
#include <math.h>
#include <string.h>

/* sceVu0Normalize (func_001252C0, an entry inside the library object at 0x1252A0): out.xyz = v.xyz
 * scaled by 1 / sqrt((x*x + y*y) + z*z), out.w = 0. A zero vector gives the VU's division by zero
 * (the largest float), times zero. */
void func_001252C0(gaddr out, gaddr v) {
    float in[3];
    memcpy(in, G(v), sizeof in);
    const float len = sqrtf((in[0] * in[0] + in[1] * in[1]) + in[2] * in[2]);
    const float q = len == 0.0f ? FLT_MAX : 1.0f / len;
    const float r[4] = {in[0] * q, in[1] * q, in[2] * q, 0.0f};
    memcpy(G(out), r, sizeof r);
}

/* sceVu0ScaleVector (func_00125328, an entry inside the library object at 0x125300): out = v * s,
 * all four components (lqc2, vmulx.xyzw by the scale, sqc2). The decompilation matches it as VU0
 * instructions in an asm statement, which a PC cannot run. */
#if !RAC1_OMIT(func_00125328)
void func_00125328(gaddr out, gaddr v, float s) {
    float in[4];
    memcpy(in, G(v), sizeof in);
    const float r[4] = {in[0] * s, in[1] * s, in[2] * s, in[3] * s};
    memcpy(G(out), r, sizeof r);
}
#endif

/* sceVu0UnitMatrix (func_00125358): the identity matrix, its rows (1,0,0,0) .. (0,0,0,1). Matched
 * as VU0 instructions in an asm statement too; without this the title world's matrices were never
 * set and it drew nothing. */
#if !RAC1_OMIT(func_00125358)
void func_00125358(gaddr m) {
    const float unit[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    memcpy(G(m), unit, sizeof unit);
}
#endif

#if !RAC1_OMIT(func_L00_002EDB68)
/* Level 0's code address 0x2EDB68: the last two instructions of func_L00_002EDB58 (`jr $31` with
 * v0 = 0 in the delay slot), which the game keeps as a callback that does nothing and returns 0. */
int func_L00_002EDB68(void) {
    return 0;
}
#endif

#if !RAC1_OMIT(func_L00_0025C710)
/* func_L00_0025C710 (mobyutil_00258BC8.c): out[i] = func_L00_0025C700(a[i], b[i], t) for four
 * floats. The matched C declares a and b as 16-byte records passed by value; the PS2's calling
 * convention hands those over by address, so every caller passes two pointers, and on a host the
 * definition and its callers would not agree. The port's version takes the pointers. Both vectors
 * are copied first, as retail copies them to its stack: out may be one of them. */
void func_L00_0025C710(gaddr out, gaddr a, gaddr b, float t) {
    float va[4];
    float vb[4];
    float r[4];
    memcpy(va, G(a), sizeof(va));
    memcpy(vb, G(b), sizeof(vb));
    for (int i = 0; i < 4; ++i) {
        r[i] = func_L00_0025C700(va[i], vb[i], t);
    }
    memcpy(G(out), r, sizeof(r));
}
#endif
