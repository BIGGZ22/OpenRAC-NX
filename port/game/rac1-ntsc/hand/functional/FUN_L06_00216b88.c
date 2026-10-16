/* Carried from the PAL decompilation's nonmatching/functional/func_L06_00216B38.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* functional: medium. Builds 4 GIF/VIF packets (a ribbon of S[5] quads per strip, strip index 3..0): each vertex of the strip at S[6] (+idx*S[5]*16) is moved to z-0.05 and z+0.05, transformed by the 4x4 matrix M, with ST pairs (u,0)/(u,1), one packed colour lerped between the colours S[3]/S[4] by the strip's S[2][idx] value, and appended at the packet pointer D_00161140 behind the templates D_L06_001C47A0/00160F40/00160F60/00160F80. VU0 maths hand-traced; cannot be run. */
extern unsigned int *D_00161140 MACRO_ADDR;
extern unsigned int D_L06_001C4AA0_16B38[] __asm__("D_L06_001C47A0");
extern unsigned int D_L06_00160F40_16B38[] __asm__("D_L06_00160EC0");
extern unsigned int D_L06_00160F60_16B38[] __asm__("D_L06_00160EE0");
extern unsigned int D_L06_00160F80_16B38[] __asm__("D_L06_00160F00");



void FUN_L06_00216b88(int *S, float *M) {
    int idx = 4;
    float step_v = 0.05f;

    do {
        float *buf1 = (float *)0x70001000;
        float *buf2 = (float *)0x70002000;
        float *vp;
        float u4, u5, step;
        int n, n2, k, val, d, c;
        unsigned int sh, col;
        float inv_rng, inv_s4, x, w, fa[4], fb[4], r;
        unsigned int *p, *start, *src, *s2;
        unsigned int cw;
        int ci;

        idx--;
        u4 = *(float *)&S[8];
        u5 = *(float *)&S[8];
        step = *(float *)&S[7];
        n = S[5];
        vp = (float *)((char *)S[6] + idx * n * 16);
        k = n;
        do {
            float vx, vy, vz, lx, ly, lz;
            u4 = u4 + step;
            u5 = u5 + step;
            vx = vp[0]; vy = vp[1]; vz = vp[2];
            lz = vz + -step_v;          /* z - 0.05 */
            vz = vz + step_v;           /* z + 0.05 */
            lx = vx; ly = vy;
            /* upper vertex (z + 0.05) goes second, lower (z - 0.05) first */
            for (c = 0; c < 4; c++) {
                buf1[c] = ((M[12 + c] * 1.0f + M[8 + c] * lz) + M[4 + c] * ly) + M[c] * lx;
                buf1[4 + c] = ((M[12 + c] * 1.0f + M[8 + c] * vz) + M[4 + c] * vy) + M[c] * vx;
            }
            buf2[0] = u4;
            buf2[1] = 0.0f;
            buf2[2] = u5;
            buf2[3] = 1.0f;
            buf1 += 8;
            buf2 += 4;
            vp += 4;
            k--;
        } while (k != 0);

        buf1 = (float *)0x70001000;
        buf2 = (float *)0x70002000;

        inv_rng = 1.0f / (float)(S[0] - S[1]);
        inv_s4 = 1.0f / (float)S[1];
        val = ((int *)S[2])[idx];
        d = val - S[1];
        if (d > 0) {
            x = (float)d * inv_rng;
        } else {
            x = (float)val * inv_s4;
            w = 1.0f - x;
            x = w;
        }
        w = 1.0f - x;
        for (c = 0; c < 4; c++) {
            fa[c] = (float)((unsigned char)((unsigned int)S[3] >> (8 * c)));
            fb[c] = (float)((unsigned char)((unsigned int)S[4] >> (8 * c)));
        }
        col = 0;
        for (c = 0; c < 4; c++) {
            r = fa[c] * w + fb[c] * x;
            ci = (int)r;
            col |= ((unsigned int)ci & 0xFF) << (8 * c);
        }

        n2 = S[5] * 2;
        sh = (unsigned int)n2 << 16;
        p = D_00161140;
        start = p;
        src = D_L06_001C4AA0_16B38;
        for (k = 0; k < 5; k++) {
            p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
            src += 4;
            p += 4;
        }
        p[-16] |= (unsigned int)n2;
        src = D_L06_00160F40_16B38;
        p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
        p += 4;
        p[-1] |= sh;
        s2 = (unsigned int *)buf2;
        for (k = 0; k < n2; k++) {
            p[0] = s2[0];
            p[1] = s2[1];
            s2 += 2;
            p += 2;
        }
        while (((unsigned int)p & 0xF) != 0) {
            *p = 0;
            p++;
        }
        src = D_L06_00160F60_16B38;
        p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
        p += 4;
        p[-1] |= sh;
        for (k = 0; k < n2; k++) {
            *p = col;
            p++;
        }
        while (((unsigned int)p & 0xF) != 0) {
            *p = 0;
            p++;
        }
        src = D_L06_00160F80_16B38;
        p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
        p += 4;
        p[-1] |= sh;
        s2 = (unsigned int *)buf1;
        for (k = 0; k < n2; k++) {
            p[0] = s2[0];
            p[1] = s2[1];
            p[2] = s2[2];
            s2 += 4;
            p += 3;
        }
        while (((unsigned int)p & 0xF) != 0) {
            *p = 0;
            p++;
        }
        p[0] = 0x15000000u | 0xE;
        p[1] = 0x11000000u;
        p[2] = 0;
        p[3] = 0;
        p += 4;
        cw = (((unsigned int)p - (unsigned int)start) >> 4) - 1;
        start[0] |= cw;
        D_00161140 = p;
    } while (idx != 0);
}
