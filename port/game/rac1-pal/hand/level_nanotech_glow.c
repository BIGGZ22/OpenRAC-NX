/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): the nanotech cluster's draw callback (class 806, func_L00_002DACC0), in C.
 *
 * Written from the retail routine, with ReRAC's notes on it (crates/rc-engine/src/fx_draw.rs, "The
 * nanotech glow"; ISC License, Copyright (c) 2026 ReRAC contributors). The cluster registers it
 * each tick it is in view (func_001F49B0 in its update). It draws three things around the
 * cluster's centre p = position + (0, 0, 0.5 + bob) + D_L00_00161BC0, in a frame that faces the
 * camera (F = unit(camera - p), R = unit(F x the crate's third row), U = R x F):
 *
 *  1. the sphere: a hemisphere of 290 vertices (D_L00_001E1810) turned to face the camera, three
 *     triangle strips (vertices 0..147, 146..254, 254..289) with per-vertex colours
 *     (D_L00_001E3340) and texture coordinates (D_L00_001E2A30), the effect texture named by
 *     D_L00_00161BA4 and ALPHA (Cs - Cd) * As + Cd: the dark translucent ball;
 *  2. the halo: 32 quads of a ring (corners D_L00_001E37D0 by D_L00_001E3DD0), inner corners at
 *     radius D_L00_00161BAC, outer at that times D_L00_00161BBC, effect texture 0xB (a radial
 *     glow), additive, its alpha pulsing with the bob;
 *  3. on a crate only (state 1): the glass's sheen, four quads of the crate box (D_L00_001E3ED0 by
 *     D_L00_001E3FD0) in the crate's frame, sphere-mapped from the view (s, t = v / 2 + 1/2, each
 *     modulo 8, v the view vector scaled to length D_L00_00161BB8 in x and y, its y taken from z).
 *
 * Retail sends the sphere's strips to VU1 (func_L00_001FDE48), which the port does not run, and
 * the quads through func_L00_001FD1D8, which the port draws natively: here the strips go through
 * func_L00_001FD1D8 too, as quads of four strip vertices (two triangles each). The GS state the
 * routine sets for the strips (TEST, TEX1, CLAMP) is the quads' own.
 *
 * Refactor notes: the strip-as-quads loop belongs in a shared helper once another callback needs
 * it (or the effect renderer could take strips from host C directly); the gp words below are the
 * level's constants and could be named once their meaning is checked in other levels.
 */
#include "common.h"

/* The level's tables (level 0's addresses; the port relocates them to the level loaded). */
extern unsigned char D_L00_001E1810[];  /* the hemisphere: 290 vertices, x y z w */
extern unsigned char D_L00_001E40F0[];  /* the vertices turned to the camera: x y z, 12 bytes */
extern unsigned char D_L00_001E3340[];  /* their colours, RGBA */
extern unsigned char D_L00_001E2A30[];  /* their texture coordinates, S T floats */
extern unsigned char D_L00_001E37D0[];  /* the halo's corner directions, x y z w */
extern unsigned char D_L00_001E3DD0[];  /* the halo's 32 quads: 4 corner indices each (shorts) */
extern unsigned char D_L00_001E3ED0[];  /* the crate glass's corners, x y z w */
extern unsigned char D_L00_001E3FD0[];  /* the glass's 4 quads: 4 corner indices each (shorts) */
extern unsigned char D_L00_00161BC0[];  /* added to the centre */
extern unsigned char D_L00_00166EC0[];  /* the camera's position */
/* The level's constants, read through $gp in retail. */
extern float D_L00_00161AE0;  /* the bob's amplitude (the alpha pulse is bob / this) */
extern int D_L00_00161BA4;    /* the sphere's effect texture */
extern float D_L00_00161BAC;  /* the halo's inner radius */
extern int D_L00_00161BB0;    /* the halo's colour (RGB) */
extern int D_L00_00161BB4;    /* the glass sheen's effect texture */
extern float D_L00_00161BB8;  /* the sheen's view vector length */
extern float D_L00_00161BBC;  /* the halo's outer radius over the inner */
extern int D_L00_00161BE0;    /* added to the halo's alpha */

extern long long func_001F4868(int id);  /* GetEffectTex: the effect texture's TEX0 */
extern void func_001F9BD8(void *out, void *a, void *b);  /* out = a + b */
extern void func_001F9BF0(void *out, void *a, void *b);  /* out = a - b */
extern void func_001F9C30(void *out, void *a, float s);  /* out.xyz = a.xyz * s, w kept */
extern void func_001F9CA0(void *out, void *a, void *b);  /* out = a x b */
extern void func_001F9EE8(void *out, void *v, void *m);  /* out = m * v (rows) */
extern void func_L00_001FD1D8(void *quad, void *matrix, int unused);  /* the callbacks' quad */
extern void func_L00_001FF4B0(void *out, void *v, float length);
extern void func_L00_001FF500(void *out, void *v, float length);
extern float func_L00_00200210(float a, float b);  /* a less the whole multiples of b */
extern int func_L00_002D95D8(void *moby);  /* the moby's state is 1 (on its crate) */
extern int func_001FA898(float f);

typedef struct GlowQuad {
    float corner[4][4];
    unsigned int rgba[4];
    float st[4][2];
    unsigned long long clamp, tex0, tex1, alpha;
} GlowQuad;

/* The sphere's strip from vertex `first`, `n` vertices, as quads of four strip vertices. */
static void sphere_strip(GlowQuad *q, int first, int n) {
    int j, k;
    for (j = 0; j + 2 < n; j += 2) {
        for (k = 0; k < 4; k++) {
            /* An odd strip's last triangle: its fourth vertex repeats the third (no area). */
            const int v = first + (j + k < n ? j + k : n - 1);
            const float *xyz = (const float *)(D_L00_001E40F0 + v * 12);
            q->corner[k][0] = xyz[0];
            q->corner[k][1] = xyz[1];
            q->corner[k][2] = xyz[2];
            q->corner[k][3] = 1.0f;
            q->rgba[k] = *(unsigned int *)(D_L00_001E3340 + v * 4);
            q->st[k][0] = *(float *)(D_L00_001E2A30 + v * 8);
            q->st[k][1] = *(float *)(D_L00_001E2A30 + v * 8 + 4);
        }
        func_L00_001FD1D8(q, 0, 0);
    }
}

void func_L00_002DACC0(unsigned char *m) {
    GlowQuad q;
    float matrix[4][4] __attribute__((aligned(16)));  /* rows F, R, U, p */
    float v[4] __attribute__((aligned(16)));
    float out[4] __attribute__((aligned(16)));
    unsigned char *pv = *(unsigned char **)(m + 0x78);
    unsigned char *box = *(unsigned char **)(pv + 0xC);  /* its crate */
    const float pulse = *(float *)pv / D_L00_00161AE0;
    int i, k;

    /* The camera-facing frame at the centre. */
    matrix[3][0] = *(float *)(m + 0x10);
    matrix[3][1] = *(float *)(m + 0x14);
    matrix[3][2] = *(float *)(m + 0x18) + 0.5f + *(float *)pv;
    matrix[3][3] = 1.0f;
    func_001F9BD8(matrix[3], matrix[3], D_L00_00161BC0);
    func_001F9BF0(v, D_L00_00166EC0, matrix[3]);
    func_L00_001FF4B0(matrix[0], v, 1.0f);
    func_001F9CA0(matrix[1], matrix[0], box + 0xE0);
    func_L00_001FF4B0(matrix[1], matrix[1], 1.0f);
    func_001F9CA0(matrix[2], matrix[1], matrix[0]);

    /* 1. The sphere. */
    for (i = 0; i < 290; i++) {
        func_001F9EE8(out, D_L00_001E1810 + i * 16, matrix);
        *(float *)(D_L00_001E40F0 + i * 12) = out[0];
        *(float *)(D_L00_001E40F0 + i * 12 + 4) = out[1];
        *(float *)(D_L00_001E40F0 + i * 12 + 8) = out[2];
    }
    q.clamp = 0;
    q.tex0 = (unsigned long long)func_001F4868(D_L00_00161BA4);
    q.tex1 = 0xFF9000000260ull;
    q.alpha = 0x8000000044ull;
    sphere_strip(&q, 0, 0x94);
    sphere_strip(&q, 146, 0x6D);
    sphere_strip(&q, 254, 0x24);

    /* 2. The halo. */
    q.tex0 = (unsigned long long)func_001F4868(0xB);
    q.alpha = 0x8000000048ull;
    {
        const unsigned int a = (unsigned int)(func_001FA898(pulse * 64.0f + 128.0f) + D_L00_00161BE0);
        q.rgba[0] = q.rgba[1] = (a << 24) | (unsigned int)D_L00_00161BB0;
        q.rgba[2] = q.rgba[3] = 0;
    }
    q.st[0][0] = q.st[0][1] = q.st[1][0] = q.st[1][1] = 0.5f;
    q.st[2][0] = q.st[3][0] = 0.5f;
    q.st[2][1] = q.st[3][1] = 1.0f;
    for (i = 0; i < 32; i++) {
        for (k = 0; k < 4; k++) {
            const short idx = *(short *)(D_L00_001E3DD0 + i * 8 + k * 2);
            const float r = k < 2 ? D_L00_00161BAC : D_L00_00161BAC * D_L00_00161BBC;
            func_001F9C30(q.corner[k], D_L00_001E37D0 + idx * 16, r);
        }
        func_L00_001FD1D8(&q, matrix, 0);
    }

    /* 3. The crate's glass sheen, while the cluster sits on its crate. */
    if (!func_L00_002D95D8(m)) {
        return;
    }
    q.tex0 = (unsigned long long)func_001F4868(D_L00_00161BB4);
    q.rgba[0] = q.rgba[1] = q.rgba[2] = q.rgba[3] = 0x107F7F7Fu;
    q.alpha = 0x8000000044ull;
    for (k = 0; k < 4; k++) {
        matrix[0][k] = *(float *)(box + 0xC0 + k * 4);
        matrix[1][k] = *(float *)(box + 0xD0 + k * 4);
        matrix[2][k] = *(float *)(box + 0xE0 + k * 4);
    }
    matrix[3][2] = *(float *)(m + 0x18);
    for (i = 0; i < 4; i++) {
        for (k = 0; k < 4; k++) {
            const short idx = *(short *)(D_L00_001E3FD0 + i * 8 + k * 2);
            func_001F9EE8(q.corner[k], D_L00_001E3ED0 + idx * 16, matrix);
            func_001F9BF0(v, q.corner[k], D_L00_00166EC0);
            v[1] = v[2];
            func_L00_001FF500(v, v, D_L00_00161BB8);
            q.st[k][0] = func_L00_00200210(v[0] * 0.5f + 0.5f, 8.0f);
            q.st[k][1] = func_L00_00200210(v[1] * 0.5f + 0.5f, 8.0f);
        }
        func_L00_001FD1D8(&q, 0, 0);
    }
}

