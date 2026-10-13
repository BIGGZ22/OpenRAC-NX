/* Carried from port/game/rac1-pal/hand/level_hero_gait.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL walk/run animation rate and phase transition (002293E8).
 * Adapted from nonmatching/shared/FUN_L00_00228c18.c, reviewed against
 * the complete 0x390-byte retail body. Sequence-table slots are pointers,
 * translated to 32-bit guest addresses by hostgen. Not a PS2 match. */
extern int func_001F9688(int);
extern void func_L00_002323B8(int, int, float);
struct HeroGaitRange { float lo, hi, c, d; };
extern struct HeroGaitRange D_L00_0017BDB8[];
extern float D_0015ED6C MACRO_ADDR;
extern int D_L00_0015F6C8[];
extern unsigned char D_0013F350[];

/* updates the hero's walk/run animation speed and gait step */
void FUN_L00_00228c18(void) {
    char *p = (char *)D_0013F350;
    int mode = *(unsigned char *)(p + 0x20A4);
    if (mode == 3) {
        float t = *(float *)(p + 0x160) * 54.0f;
        *(float *)(p + 0xA90) = t;
        if (t < 0.7f) *(float *)(p + 0xA90) = 0.7f;
    } else if (mode == 1) {
        float t = *(float *)(p + 0x160) * 90.0f;
        *(float *)(p + 0xA90) = t;
        if (t < 0.5f) *(float *)(p + 0xA90) = 0.5f;
    } else {
        int flag, idx, old, n, i;
        char *q, *r;
        float *hp;
        if (func_001F9688(4) < *(int *)(p + 0x1A0) && *(short *)(p + 0x3BC) == 0) {
            idx = *(int *)(p + 0x2088);
            flag = *(float *)(p + 0x194) < D_L00_0017BDB8[idx].lo * D_0015ED6C;
            if (flag || D_L00_0017BDB8[idx].hi * D_0015ED6C < *(float *)(p + 0x194)) {
                if (func_001F9688(4) < *(int *)(p + 0x1A0) && *(short *)(p + 0x3BC) == 0) {
                    old = *(int *)(p + 0x2088);
                    if (flag) {
                        if (*(float *)(p + 0x194) < D_L00_0017BDB8[old].lo * D_0015ED6C) {
                            char *pd;
                            struct HeroGaitRange *td;
                            pd = p;
                            td = D_L00_0017BDB8;
                            do {
                                *(int *)(pd + 0x2088) -= 1;
                            } while (*(float *)(pd + 0x194) < (td = D_L00_0017BDB8)[*(int *)(pd + 0x2088)].lo * D_0015ED6C);
                        }
                    } else {
                        hp = &D_L00_0017BDB8[0].hi;
                        if (hp[old * 4] * D_0015ED6C < *(float *)(p + 0x194)) {
                            float *hq;
                            char *pu;
                            i = old;
                            i++;
                            while ((hq = hp)[i * 4] * D_0015ED6C < *(float *)((pu = p) + 0x194)) {
                                i++;
                            }
                            *(int *)(pu + 0x2088) = i;
                        }
                    }
                    q = (char *)D_0013F350;
                    if (*(int *)(q + 0x2088) < 0) *(int *)(q + 0x2088) = 0;
                    if (*(int *)(q + 0x2088) > 1) *(int *)(q + 0x2088) = 1;
                    n = *(int *)(q + 0x2088);
                    if (old != n) {
                        int k = n + 3;
                        int j = old + 3;
                        char *o = *(char **)(q + 0x2080);
                        unsigned char **tb = (unsigned char **)(*(char **)(o + 0x24) + 0x48);
                        int e1 = *(unsigned char *)(o + 0x51);
                        unsigned char *a = tb[k];
                        unsigned char *c = tb[j];
                        int tv = D_L00_0015F6C8[n + old * 2];
                        int rr = ((e1 * a[0x10]) / c[0x10] + tv) % a[0x10];
                        func_L00_002323B8(k, rr, (float)func_001F9688(8));
                    }
                }
            }
        }
        r = (char *)D_0013F350;
        switch (*(int *)(r + 0x2088)) {
        case 0: {
            float t = *(float *)(r + 0x194) * 157.0f;
            *(float *)(r + 0xA90) = t;
            if (t < 0.6f) *(float *)(r + 0xA90) = 0.6f;
            if (4.0f < *(float *)(r + 0xA90)) *(float *)(r + 0xA90) = 4.0f;
            break;
        }
        case 1: {
            float t = *(float *)(r + 0x194) * 14.0f;
            *(float *)(r + 0xA90) = t;
            if (t < 0.6f) *(float *)(r + 0xA90) = 0.6f;
            if (2.2f < *(float *)(r + 0xA90)) *(float *)(r + 0xA90) = 2.2f;
            break;
        }
        }
    }
}
