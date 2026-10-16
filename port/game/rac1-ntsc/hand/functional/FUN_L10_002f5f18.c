/* Carried from the PAL decompilation's src/overlays/shared/vendor_00299AF0.c (matched C there) (games/rac1) through the version
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
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9BD8_99AF0(void *, void *, void *) __asm__("func_001F9A10");
extern int func_001F9688(int);
extern int func_001FA6D0(float);
extern float func_001F9B48(void *, void *);
extern char D_0013E533[];
extern float D_0015ED6C MACRO_ADDR;
extern void func_001F9A68(void *, void *, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern char *D_L10_0015EF50 MACRO_ADDR;
extern float D_L10_0015F3FC MACRO_ADDR;
extern char *D_L10_0015FFD8 MACRO_ADDR;
extern char *D_L10_001B0930[];
extern int D_L10_0015F404 MACRO_ADDR;
extern float D_L10_0016CF70;
extern float D_L10_00167358;
extern char D_L10_0016CEC0[];
extern float D_L10_00167450[];
extern void func_001F9CF8(void *, void *, void *);
extern void func_001F9AD8(void *, void *, void *);
extern void func_001F2D98(void);
extern void func_L00_002223F8(int, int);
extern int func_L00_0025D808(void *, void *, int *, float *, float, int);
extern int func_001F9770(void *);
extern float func_002133D0(float, float, float);
extern void func_001E93E8(void *, void *);
struct CamK_72D8_c2f5f18 {
    short ids[8];
    float vals[9];
};



/* Camera 19 update: advances the blend state and aims the camera each frame. */
void FUN_L10_002f5f18(char *moby) {
    float va[4];
    float v1[4];
    float v2[4];
    float v3[4];
    int ia;
    float ib;
    char *dat;
    char *f;
    struct CamK_72D8_c2f5f18 *k;
    char *e;
    char *g;
    short st;
    float t;

    dat = *(char **)(moby + 0x70);
    f = dat + 0x40;
    k = (struct CamK_72D8_c2f5f18 *)(dat + 0x58);
    e = *(char **)(D_L10_0015EF50 + *(short *)(moby + 0x84) * 0x20 + 0x1C);
    st = *(short *)(f + 0x14);

    if (st == 0) {
        char *a;
        char *b;
        char *pm;
        char *gb;
        float *c3;
        D_L10_0015F3FC += 0.05f;
        if (1.0f <= D_L10_0015F3FC) {
            a = D_L10_001B0930[*(int *)(e + 0x20)];
            b = D_L10_001B0930[*(int *)(e + 0x24)];
            pm = moby + 0x30;
            D_L10_0015F3FC = 1.0f;
            a = a + 0x10;
            D_L10_0015F404 = 1;
            *(short *)(f + 0x14) = 1;
            qcopy(pm, a);
            if (*(int *)(e + 0x28) >= 0) {
                gb = D_L10_0015FFD8 + (*(int *)(e + 0x28) << 8);
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
        return;
    }
    if (st == 2) {
        float x;
        float aa;
        float bb;
        D_L10_0015F3FC += 0.05f;
        if (1.0f <= D_L10_0015F3FC) {
            *(short *)(f + 0x14) = 3;
            D_L10_0015F3FC = 1.0f;
            x = k->vals[8] * 0.5f;
            aa = func_001F9DE0(x);
            bb = func_001F9DC8(x);
            D_L10_0016CF70 = aa / bb;
            func_001F2D98();
            *(unsigned char *)(e + 0x39) = 1;
            D_L10_00167358 = 0.05f;
            *(short *)(moby + 0x7E) = 4;
            D_L10_0015F404 = 0;
            if (*(int *)(e + 0x4C) == 0) {
                func_L00_002223F8(0, 1);
            }
        }
        if (*(int *)(e + 0x40) == 0) {
            return;
        }
    } else if (st == 3) {
        return;
    } else {
        D_L10_0015F3FC -= 0.05f;
        if (D_L10_0015F3FC < 0.0f) {
            D_L10_0015F3FC = 0.0f;
        }
    }

    {
        float d;
        float nv;
        int rr;
        int i;

        g = D_L10_001B0930[*(int *)(e + 0x20)];
        qcopy(va, (moby + 0x30));
        rr = func_L00_0025D808(g, (moby + 0x30), (int *)f, (float *)(f + 4), *(float *)(e + 0x3C) * D_0015ED6C, 0);
        if (*(short *)(f + 0x16) != 0) {
            if (func_001F9770(f + 0x16) != 0) {
                rr = 1;
            } else if (*(int *)(e + 0x40) != 0) {
                short s16 = *(short *)(f + 0x16);
                if (s16 < func_001F9688(func_001FA6D0(20.0f))) {
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
        for (i = 0; i < 7 && k->ids[i] >= 0 && k->ids[i + 1] >= 0; i++) {
            if (*(int *)f >= k->ids[i] && *(int *)f < k->ids[i + 1]) {
                char *cam;
                float sum = 0.0f;
                float fv = *(float *)(f + 4);
                float total;
                float r;
                float m;
                float aa;
                float bb;
                int j;
                int j2;
                for (j = k->ids[i]; j < *(int *)f; j++) {
                    sum = sum + *(float *)(g + j * 16 + 0x1C);
                }
                total = sum;
                sum = sum + fv;
                for (j2 = *(int *)f; j2 < k->ids[i + 1]; j2++) {
                    total = total + *(float *)(g + j2 * 16 + 0x1C);
                }
                r = func_002133D0(k->vals[i], k->vals[i + 1], sum / total);
                m = r * 0.017453292f * 0.5f;
                aa = func_001F9DE0(m);
                bb = func_001F9DC8(m);
                cam = D_L10_0016CEC0;
                *(float *)(cam + 0xB0) = aa / bb;
                func_001F2D98();
            }
        }
        d = func_001F9B48(va, (moby + 0x30));
        nv = *(float *)(f + 0x10) + d;
        *(float *)(f + 0x10) = nv;
        if (*(int *)(e + 0x28) >= 0) {
            char *b17 = D_L10_0015FFD8 + (*(int *)(e + 0x28) << 8);
            float *c3 = (float *)(e + 0x2C);
            v2[0] = c3[0];
            v2[1] = c3[1];
            v2[2] = c3[2];
            v2[3] = 0.0f;
            func_001F9CF8(v2, v2, D_L10_00167450);
            func_001F9BD8_99AF0(v3, b17 + 0x10, v2);
            func_001F9A28(v1, v3, (moby + 0x30));
        } else {
            char *h;
            d = (nv / *(float *)(f + 8)) * *(float *)(f + 0xC);
            h = D_L10_001B0930[*(int *)(e + 0x24)];
            ia = 0;
            ib = 0.0f;
            func_L00_0025D808(h, v2, &ia, &ib, d, 0);
            ((void (*)(void *, float))func_001E93E8)(v2, 0.2f);
            func_001E93E8(h + 0x10, h + (*(int *)h << 4));
            func_001F9A28(v1, v2, (moby + 0x30));
        }
        func_001F9A68(v2, D_0013E533 + 0x10AD, -1.0f);
        func_L00_001FF500(moby, v1, 1.0f);
        func_001F9AD8(moby + 0x10, moby, v2);
        func_L00_001FF500(moby + 0x10, moby + 0x10, 1.0f);
        func_001F9AD8(moby + 0x20, moby + 0x10, moby);
    }
}
