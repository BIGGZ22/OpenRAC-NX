/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/* The 3x3 rows multiply: out (3 rows of 4 floats) = A * B, A the 4x3 block at src (vf4..vf6),
 * B the block at b (vf1..vf3); each lane out[j][i] = (A0[i]*B[j][0] + A1[i]*B[j][1]) +
 * A2[i]*B[j][2]. The routine loads all six quadwords before its first store, so out may be src
 * or b: callers rely on it (a moby's own rows times its parent's, out == b: the Pyrocitor's pilot
 * flame, pause.c's moby basis). Both inputs are copied first for that. This replaces the
 * decompilation's functional C (nonmatching/game/func_001FA4F0.c), which stores lane by lane and
 * reads overwritten lanes when out == b. */
void func_001FA4F0(float *out, const float *src, const float *b) {
    float a[12], m[12];
    int j, i;
    for (i = 0; i < 12; i++) {
        a[i] = src[i];
        m[i] = b[i];
    }
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 4; i++) {
            float t = a[i] * m[4 * j + 0];
            t = t + a[4 + i] * m[4 * j + 1];
            out[4 * j + i] = t + a[8 + i] * m[4 * j + 2];
        }
    }
}
