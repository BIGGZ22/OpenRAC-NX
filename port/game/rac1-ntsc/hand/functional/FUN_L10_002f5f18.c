/* Carried from the PAL decompilation's nonmatching/shared/func_L10_002F72D8.c (games/rac1/pal) through the version
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
/* What the PAL file this was written in declares before it. */
extern char D_0013E533[];

/* NON_MATCHING FUN_L10_002f5f18 -- src/overlays/shared/vendor_00299AF0.c
 * Best so far: SIZE ours 1752 / retail 1756, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Camera 19 update (levels 10/13/14/15): state machine on the short at f+0x14 (0 idle, 1 blend, 2 aim with a 0.1
 *   Difference: the callee-saved set and moby's register differ (retail moby $s6, saves $s0-$s7 and $fp; ours moby
 */
extern char *D_L10_0015EF50 MACRO_ADDR;
extern float D_L10_0015F3FC MACRO_ADDR;
extern char *D_L10_0015FFD8 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern char *D_L10_001B0930[];
extern int D_L10_0015F404;
extern float D_L10_0016CF70;
extern float D_L10_00167358;
extern char D_L10_0016CEC0[];
extern float D_L10_00167450[] MACRO_ADDR;
extern void func_001F9CF8(void *, void *, void *);
extern void func_001F9BD8_99AF0(void *, void *, void *) __asm__("func_001F9A10");
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9AD8(void *, void *, void *);
extern float func_001F9DE0(float);
extern float func_001F9DC8(float);
extern void func_001F2D98(void);
extern void func_L00_002223F8(int, int);
extern int func_L00_0025D808(void *, void *, int *, float *, float, int);
extern int func_001F9770(void *);
extern int func_001FA6D0(float);
extern int func_001F9688(int);
extern float func_002133D0(float, float, float);
extern float func_001F9B48(void *, void *);
extern void func_001E93E8(void *, void *);

/* Camera 19 update: advances the blend state and aims the camera each frame. */
void FUN_L10_002f5f18(char *moby) {
    float va[4];
    float v1[4];
    float v2[4];
    float v3[4];
    int ia;
    float ib;
    char *pm;
    char *dat;
    char *f;
    char *k;
    char *e;
    char *g;
    char *h;
    char *q;
    char *a;
    char *b;
    char *gb;
    char *b17;
    float *c3;
    float *kv;
    short *kk;
    short st;
    short s16;
    float t;
    float nv;
    float d;
    float sum;
    float s0;
    float s1;
    float fv;
    float w;
    float r;
    float m;
    float aa;
    float bb;
    float x;
    int rr;
    int go;
    int i;
    int F;
    int n;
    int v;

    dat = *(char **)(moby + 0x70);
    k = dat + 0x58;
    f = dat + 0x40;
    e = *(char **)(D_L10_0015EF50 + *(short *)(moby + 0x84) * 0x20 + 0x1C);
    pm = moby + 0x30;
    st = *(short *)(f + 0x14);
    go = 0;

    if (st == 0) {
        t = D_L10_0015F3FC + 0.1f;
        D_L10_0015F3FC = t;
        if (1.0f <= t) {
            a = D_L10_001B0930[*(int *)(e + 0x20)];
            b = D_L10_001B0930[*(int *)(e + 0x24)];
            D_L10_0015F3FC = 1.0f;
            a = a + 0x10;
            D_L10_0015F404 = 1;
            *(short *)(f + 0x14) = 1;
            qcopy(pm, a);
            gb = D_L10_0015FFD8;
            if (*(int *)(e + 0x28) >= 0) {
                gb = gb + (*(int *)(e + 0x28) << 8);
                c3 = (float *)(e + 0x2C);
                v1[0] = c3[0];
                v1[1] = c3[1];
                v1[2] = c3[2];
                v1[3] = 0.0f;
                func_001F9CF8(v1, v1, D_L10_00167450);
                func_001F9BD8_99AF0(v2, gb + 0x10, v1);
                func_001F9A28(va, v2, pm);
            } else {
                func_001F9A28(va, b + 0x10, pm);
            }
            func_001F9A68(v1, D_0013E533 + 0x10AD, -1.0f);
            func_L00_001FF500(moby, va, 1.0f);
            func_001F9AD8(moby + 0x10, moby, v1);
            func_L00_001FF500(moby + 0x10, moby + 0x10, 1.0f);
            func_001F9AD8(moby + 0x20, moby + 0x10, moby);
        }
    } else if (st == 2) {
        t = D_L10_0015F3FC + 0.1f;
        D_L10_0015F3FC = t;
        if (1.0f <= t) {
            *(short *)(f + 0x14) = 3;
            D_L10_0015F3FC = 1.0f;
            x = *(float *)(k + 0x30) * 0.5f;
            aa = func_001F9DE0(x);
            bb = func_001F9DC8(x);
            D_L10_0016CF70 = aa / bb;
            func_001F2D98();
            *(unsigned char *)(e + 0x39) = 1;
            D_L10_00167358 = 0.1f;
            *(short *)(moby + 0x7E) = 4;
            D_L10_0015F404 = 0;
            if (*(int *)(e + 0x4C) == 0) {
                func_L00_002223F8(0, 1);
            }
        }
        if (*(int *)(e + 0x40) != 0) {
            go = 1;
        }
    } else if (st != 3) {
        t = D_L10_0015F3FC - 0.1f;
        D_L10_0015F3FC = t;
        if (t < 0.0f) {
            D_L10_0015F3FC = 0.0f;
        }
        go = 1;
    }

    if (go) {
        g = D_L10_001B0930[*(int *)(e + 0x20)];
        qcopy(va, pm);
        rr = func_L00_0025D808(g, pm, (int *)f, (float *)(f + 4), *(float *)(e + 0x3C) * D_0015ED6C, 0);
        if (*(short *)(f + 0x16) != 0) {
            if (func_001F9770(f + 0x16) != 0) {
                rr = 1;
            } else if (*(int *)(e + 0x40) != 0) {
                s16 = *(short *)(f + 0x16);
                v = func_001F9688(func_001FA6D0(20.0f));
                if (s16 < v) {
                    rr = 1;
                }
            }
        }
        if (*(unsigned char *)(e + 0x39) != 0) {
            rr = 1;
        }
        if (rr != 0) {
            *(short *)(f + 0x14) = 2;
            if (*(int *)(e + 0x40) == 0) {
                return;
            }
        }
        kv = (float *)(k + 0x10);
        kk = (short *)k;
        if (kk[0] >= 0 && kk[1] >= 0) {
            i = 0;
            for (;;) {
                F = *(int *)f;
                if (F >= kk[i] && F < kk[i + 1]) {
                    sum = 0.0f;
                    fv = *(float *)(f + 4);
                    if (kk[i] < F) {
                        n = F - kk[i];
                        q = g + kk[i] * 16 + 0x1C;
                        do {
                            w = *(float *)q;
                            n = n - 1;
                            q = q + 16;
                            sum = sum + w;
                        } while (n != 0);
                    }
                    s1 = sum;
                    s0 = sum + fv;
                    if (F < kk[i + 1]) {
                        n = kk[i + 1] - F;
                        q = g + F * 16 + 0x1C;
                        do {
                            w = *(float *)q;
                            n = n - 1;
                            q = q + 16;
                            s1 = s1 + w;
                        } while (n != 0);
                    }
                    r = func_002133D0(kv[i], kv[i + 1], s0 / s1);
                    m = r * 0.017453292f * 0.5f;
                    aa = func_001F9DE0(m);
                    bb = func_001F9DC8(m);
                    *(float *)(D_L10_0016CEC0 + 0xB0) = aa / bb;
                    func_001F2D98();
                }
                if (!(i + 1 < 7 && kk[i + 1] >= 0 && kk[i + 2] >= 0)) {
                    break;
                }
                i = i + 1;
            }
        }
        d = func_001F9B48(va, pm);
        nv = *(float *)(f + 0x10) + d;
        *(float *)(f + 0x10) = nv;
        if (*(int *)(e + 0x28) < 0) {
            h = D_L10_001B0930[*(int *)(e + 0x24)];
            ia = 0;
            ib = 0.0f;
            func_L00_0025D808(h, v2, &ia, &ib, (nv / *(float *)(f + 8)) * *(float *)(f + 0xC), 0);
            ((void (*)(void *, float))func_001E93E8)(v2, 0.2f);
            func_001E93E8(h + 0x10, h + (*(int *)h << 4));
            func_001F9A28(v1, v2, pm);
        } else {
            b17 = D_L10_0015FFD8 + (*(int *)(e + 0x28) << 8);
            c3 = (float *)(e + 0x2C);
            v2[0] = c3[0];
            v2[1] = c3[1];
            v2[2] = c3[2];
            v2[3] = 0.0f;
            func_001F9CF8(v2, v2, D_L10_00167450);
            func_001F9BD8_99AF0(v3, b17 + 0x10, v2);
            func_001F9A28(v1, v3, pm);
        }
        func_001F9A68(v2, D_0013E533 + 0x10AD, -1.0f);
        func_L00_001FF500(moby, v1, 1.0f);
        func_001F9AD8(moby + 0x10, moby, v2);
        func_L00_001FF500(moby + 0x10, moby + 0x10, 1.0f);
        func_001F9AD8(moby + 0x20, moby + 0x10, moby);
    }
}
