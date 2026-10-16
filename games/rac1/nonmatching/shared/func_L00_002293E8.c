/* NON_MATCHING func_L00_002293E8 -- src/overlays/shared/help_00221A98.c
 * Best so far: SIZE ours 908 / retail 912, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_002293E8 (HeroWalkRunAnim, shared, 912 bytes): sets the hero animation speed (p+0xA90) from speed and
 *   Not exact: size 892 vs 912 with p7.c (904 with the for-loop variant p6.c), 10 runs spent. Structure, top clamp
 *   Difference: retail's two index loops carry extra register copies (p, table base and old copied into fresh regi
 *   Hero walk/run anim speed + gait step. Remaining diff is the upward index loop and the gait-step table setup: r
 *   hq13/n02 (8 runs, p11-p17): p15.c is the best by size (908/912): the gait-step block now matches exactly (j = 
 */
#include "common.h"
extern int func_001F9850(int);
extern void func_L00_00232C10(int, int, float);
struct T { float lo, hi, c, d; };
extern struct T D_L00_0017BEB8[];
extern float D_0015EE6C MACRO_ADDR;
extern short D_L00_0015F7A8;
extern unsigned char D_0013E633[] NOT_SDA;

/* updates the hero's walk/run animation speed and gait step */
void func_L00_002293E8(void) {
    char *p = (char *)D_0013E633 + 0xE1D;
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
        if (func_001F9850(4) < *(int *)(p + 0x1A0) && *(short *)(p + 0x3BC) == 0) {
            idx = *(int *)(p + 0x2088);
            flag = *(float *)(p + 0x194) < D_L00_0017BEB8[idx].lo * D_0015EE6C;
            if (flag || D_L00_0017BEB8[idx].hi * D_0015EE6C < *(float *)(p + 0x194)) {
                if (func_001F9850(4) < *(int *)(p + 0x1A0) && *(short *)(p + 0x3BC) == 0) {
                    old = *(int *)(p + 0x2088);
                    if (flag) {
                        if (*(float *)(p + 0x194) < D_L00_0017BEB8[old].lo * D_0015EE6C) {
                            char *pd;
                            struct T *td;
                            pd = p;
                            td = D_L00_0017BEB8;
                            do {
                                *(int *)(pd + 0x2088) -= 1;
                            } while (*(float *)(pd + 0x194) < (td = D_L00_0017BEB8)[*(int *)(pd + 0x2088)].lo * D_0015EE6C);
                        }
                    } else {
                        hp = &D_L00_0017BEB8[0].hi;
                        if (hp[old * 4] * D_0015EE6C < *(float *)(p + 0x194)) {
                            float *hq;
                            char *pu;
                            i = old;
                            i++;
                            while ((hq = hp)[i * 4] * D_0015EE6C < *(float *)((pu = p) + 0x194)) {
                                i++;
                            }
                            *(int *)(pu + 0x2088) = i;
                        }
                    }
                    q = (char *)D_0013E633 + 0xE1D;
                    if (*(int *)(q + 0x2088) < 0) *(int *)(q + 0x2088) = 0;
                    if (*(int *)(q + 0x2088) > 1) *(int *)(q + 0x2088) = 1;
                    n = *(int *)(q + 0x2088);
                    if (old != n) {
                        int k = n + 3;
                        int j = old + 3;
                        char *o = *(char **)(q + 0x2080);
                        int *tb = *(int **)(o + 0x24) + 0x12;
                        int e1 = *(unsigned char *)(o + 0x51);
                        unsigned char *a = (unsigned char *)tb[k];
                        unsigned char *c = (unsigned char *)tb[j];
                        int tv = *(int *)((char *)&D_L00_0015F7A8 + n * 4 + old * 8);
                        int rr = ((e1 * a[0x10]) / c[0x10] + tv) % a[0x10];
                        func_L00_00232C10(k, rr, (float)func_001F9850(8));
                    }
                }
            }
        }
        r = (char *)D_0013E633 + 0xE1D;
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
