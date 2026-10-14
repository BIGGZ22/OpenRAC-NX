/* Carried from the PAL decompilation's nonmatching/l06_blarg/func_L06_00304B88.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L06_00303748 -- src/overlays/l06_blarg/vendor_002FE5D0.c
 * Best so far: SIZE ours 1072 / retail 1056, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Two passes: a counted loop of 4 helper calls per pass (func_00213260, func_001F9688, func_L00_00257B90, then f
 *   Best p2.c: 1072 of 1056 bytes (16 over). Control flow and calls match; the allocator differs: retail keeps the
 */
extern void func_L00_0025F090(void *, void *, int, float, float);
extern int func_00213260(int);
extern int func_001F9688(int);
extern int func_L00_00257B90(int, int);
extern void func_L00_0026A9F0(void *, void *, int, int, float, int, int, int, int, float);
extern char *func_00218888(void *, void *, void *, int, int, int, int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern float func_002132A8(float, float);
extern void func_001F9A68(void *, void *, float);
extern int func_001FA6E0(int, int, float);
extern float func_001F96B0(float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_L00_00257D78(float *, float, float);
extern float D_0015ED6C MACRO_ADDR;
extern char D_L06_00201948[];
extern char D_L06_00201960[];
extern short D_L06_00162114;
extern short D_L06_00162118;
extern short D_L06_0016211C;
extern short D_L06_00162120;
extern short D_L06_00162124;
extern short D_L06_00162128;
extern short D_L06_0016212C;
extern short D_L06_00162130;
extern short D_L06_00162134;
extern short D_L06_00162138;
extern short D_L06_0016213C;
extern short D_L06_00162140;
extern short D_L06_00162144;
extern short D_L06_00162148;
extern short D_L06_0016214C;
extern short D_L06_00162150;
extern short D_L06_00162154;
extern short D_L06_00162158;

typedef struct { char b[24]; } __attribute__((packed)) P24_c303748;

// Two passes over sampled tables: a per-count loop that calls the moby effect helper, then a loop of smoothed vector samples fed to func_00218888
void FUN_L06_00303748(void *a0, void *a1, void *a2, float f, int n) {
    int A[6];
    int B[6];
    float v4[4];
    float v1[4];
    float v2[4];
    float v3[4];
    int i, c, m;
    int *p18;
    int *p17;
    float g;
    int r1, r2, r16, r5, r19, r9;
    int g1, g2, x1, x2, x3;
    float q3, q5, q7, q8;
    float t1, t2;

    func_L00_0025F090(a0, a1, -1, 2.0f, 13.0f);
    *(P24_c303748 *)A = *(P24_c303748 *)D_L06_00201948;
    *(P24_c303748 *)B = *(P24_c303748 *)D_L06_00201960;

    for (c = n; c > 0; c--) {
        g = f * 400000.0f;
        r1 = func_00213260(6);
        p18 = &A[r1];
        r2 = func_00213260(6);
        p17 = &B[r2];
        r16 = func_001F9688(0xF);
        r5 = func_001F9688(0x14);
        r19 = func_L00_00257B90(r16, r5);
        r16 = func_001F9688(0x19);
        r5 = func_001F9688(0x1E);
        r9 = func_L00_00257B90(r16, r5);
        func_L00_0026A9F0(a1, a2, *p18, *p17, g, r19, r9, 0, 0, D_0015ED6C * 10.0f * f);
    }

    m = *(int *)&D_L06_00162150 * n;
    if (m > 0) {
        for (i = 0; i < *(int *)&D_L06_00162150 * n; i++) {
            func_L00_00257D78(v4, 0.0f, 1.0f);
            func_001F9A10(v4, v4, a1);
            func_L00_00257D78(v1, 0.0f, 0.9f);
            func_001F9A10(v1, v1, a2);
            func_001F9A68(v2, v1, func_002132A8(*(float *)&D_L06_00162114, *(float *)&D_L06_00162118) * D_0015ED6C);
            func_L00_00257D78(v1, 0.0f, 0.9f);
            func_001F9A10(v1, v1, a2);
            func_001F9A68(v3, v1, func_002132A8(*(float *)&D_L06_00162124, *(float *)&D_L06_00162128) * D_0015ED6C);
            t1 = func_002132A8(*(float *)&D_L06_0016211C, *(float *)&D_L06_00162120) * f;
            v2[3] = t1;
            t2 = func_002132A8(*(float *)&D_L06_0016212C, *(float *)&D_L06_00162130) * f;
            v3[3] = t2;
            q3 = func_002132A8(0.0f, 1.0f);
            g1 = func_001FA6E0(*(int *)&D_L06_00162134, *(int *)&D_L06_00162138, q3);
            q5 = func_002132A8(0.0f, 1.0f);
            g2 = func_001FA6E0(*(int *)&D_L06_0016213C, *(int *)&D_L06_00162140, q5);
            q7 = func_002132A8((float)*(int *)&D_L06_00162144, (float)(*(int *)&D_L06_00162144 * 2));
            x1 = func_001FA898_r(func_001F96B0(q7));
            q8 = func_002132A8((float)*(int *)&D_L06_00162148, (float)*(int *)&D_L06_00162148 * 1.2f);
            x2 = func_001FA898_r(func_001F96B0(q8));
            q3 = func_002132A8((float)*(int *)&D_L06_0016214C * 0.8f, (float)*(int *)&D_L06_0016214C);
            x3 = func_001FA898_r(func_001F96B0(q3));
            func_00218888(v4, v2, v3, g1, g2, x1, x2, x3, *(int *)&D_L06_00162154 + (*(int *)&D_L06_00162158 << 16));
        }
    }
}
