/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002BD6B8.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L00_002bc3b0 -- src/overlays/shared/vendor_002BA7C8.c
 * Best so far: SIZE ours 1060 / retail 1076, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Scans the moby list at D_L00_001ABA00 for the nearest target to a1 (float math through matched helpers), write
 *   Left: the early code differs (retail stores a0 to sp+0x40 and keeps the result at sp+0x44 in memory; ours keep
 *   Unblock: a form of the entry copy and the result that gives retail's frame slots, then the placement of the r 
 */
extern unsigned char *func_L00_0025C338(int);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B80(void *, void *);
extern float func_001FA688(float, float);
extern float func_001F9B48(void *, void *);
extern float func_001F99C0(float);
extern void func_00214DB0(void *, float, float, float);
extern void func_001F9A10(void *, void *, void *);
extern float func_001F9DF8(float x);
extern float func_001FA6C0(int);
extern s32 func_L00_001EFFF0_C0358(void *, void *, s32, void *, s32) __asm__("func_L00_001EFC70");
extern char *D_L00_001ABA00[];
extern int D_L00_00173E40[];
extern char D_L00_00173F60_v[] __asm__("D_L00_00173E60") MACRO_ADDR;
extern float D_L00_001616DC SDATA(D_L00_001616DC);
extern float D_L00_001616D8 SDATA(D_L00_001616D8);
extern char D_0013E533[];
typedef int u128_BD6B8_c2bc3b0 __attribute__((mode(TI)));

/* Scans the moby list for the target nearest a1; sets a2/a3 for the hit and returns that moby, else 0. */
int FUN_L00_002bc3b0(char *a0, float *a1, float *a2, float *a3) {
    float vec[4];
    float v10[4];
    float v20[4];
    float v30[4];
    float f20, f21, f22, f23, f24, f25, f26, t, r;
    char *obj;
    char *g;
    char *obj2;
    char *p;
    char **k;
    int ok;
    int a0s[1];
    int res[1];

    f26 = D_L00_001616DC;
    a0s[0] = (int)a0;
    g = D_0013E533 + 0xE1D;
    *(u128_BD6B8_c2bc3b0 *)vec = *(u128_BD6B8_c2bc3b0 *)a1;
    obj2 = *(char **)(g + 0x2080);
    qcopy(v10, obj2 + 0x10);
    v10[2] = vec[2];
    res[0] = 0;
    if (func_L00_001EFFF0_C0358(v10, vec, 6, obj2, 0)) {
        *(u128_BD6B8_c2bc3b0 *)vec = *(u128_BD6B8_c2bc3b0 *)D_L00_00173F60_v;
        return 0;
    }
    for (k = D_L00_001ABA00; (obj = *k) != 0; k++) {
        if ((signed char)obj[0x20] < 0) continue;
        if (*(short *)(obj + 0xA6) == 0x58E) continue;
        if (*(short *)(obj + 0xA6) == 0x452) continue;
        qcopy(v20, obj + 0x10);
        p = (char *)func_L00_0025C338((int)obj);
        if (!p) continue;
        v20[2] = v20[2] + *(float *)(p + 0x10);
        if (!*(char **)(obj + 0x24) || *(short *)(*(char **)(obj + 0x24) + 0x46) != 5) continue;
        f23 = func_L00_001FF8B0(v20[0] - vec[0], v20[1] - vec[1]);
        t = func_001F9B80(vec, v20);
        f24 = func_L00_001FF8B0(t, v20[2] - vec[2]);
        f25 = func_001FA688(*a2, f23);
        f22 = func_001F9B48(vec, v20);
        if (f22 < 2.5f) {
            float x = *(float *)(obj + 0x10);
            t = func_L00_001FF8B0(x - *(float *)(g + 0x80), *(float *)(obj + 0x14) - *(float *)(g + 0x84));
            t = func_001FA688(*(float *)(*(char **)(g + 0x2080) + 0x48), t);
            if (t < 1.04719758f) {
                t = func_001F99C0(vec[2] - v20[2]);
                if (t < 2.0f) {
                    if (func_L00_001EFFF0_C0358(vec, v20, 6, *(void **)(g + 0x2080), 0) == 0) {
                        a3[0] = -f24;
                        a2[0] = f23;
                        res[0] = (int)obj;
                        return res[0];
                    }
                }
            }
        }
        if (f26 < f22) continue;
        func_00214DB0(v30, f22, a2[0], a3[0] + *(float *)(g + 0x2E4) * 0.5f);
        func_001F9A10(v30, v30, vec);
        r = func_001F9B48(v30, v20);
        f21 = func_001F9DF8(r / (f22 + f22));
        f21 = f21 + f21;
        if (0.0174532924f * D_L00_001616D8 < f21) {
            p = (char *)func_L00_0025C338((int)obj);
            if (p) {
                f20 = func_001FA6C0(*(unsigned char *)(p + 0xA)) * 0.125f;
                r = func_001F9DF8(f20 / f22);
                if (f20 < f22) {
                    f21 = f21 - r;
                    if (f21 < 0.0f) f21 = 0.0f;
                }
            }
        }
        if (f21 < D_L00_001616D8 * 0.0174532924f) {
            ok = 1;
            if (func_L00_001EFFF0_C0358(vec, v20, 6, (void *)a0s[0], 0)) {
                int x2 = D_L00_00173E40[6];
                if (x2 == 0 || x2 != (int)obj) ok = 0;
            }
            if (ok) {
                f26 = f22;
                a3[0] = -f24;
                if (f25 < 0.17453292f) a2[0] = f23;
                res[0] = (int)obj;
            }
        }
    }
    return res[0];
}
