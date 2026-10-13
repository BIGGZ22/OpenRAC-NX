/* Carried from the PAL decompilation's nonmatching/l10_orxon/func_L10_002E4E50.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L10_002e3a90 -- src/overlays/l10_orxon/vendor_002E30F8.c
 * Best so far: SIZE ours 840 / retail 856, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Per-frame update of the moby in arg0: copies arg0+0x10 into a vector, scales two trig-derived vectors (a, b) f
 *   Stopped at a wall (QUEUE.md): retail reloads D_0015ED6C with a fresh `lui $1, %hi` and `lwc1` before each use 
 */
extern int D_L10_00161E74 SDATA(D_L10_00161E74);
extern int D_L10_00161E78 SDATA(D_L10_00161E78);
extern float D_L10_00161E7C SDATA(D_L10_00161E7C);
extern float D_L10_00161E80 SDATA(D_L10_00161E80);
extern float D_L10_00161E84 SDATA(D_L10_00161E84);
extern float D_L10_00161E88 SDATA(D_L10_00161E88);
extern float D_L10_00161E8C SDATA(D_L10_00161E8C);
extern float D_L10_00161E90 SDATA(D_L10_00161E90);
extern float D_L10_00161E94 SDATA(D_L10_00161E94);
extern float D_L10_00161E98 SDATA(D_L10_00161E98);
extern float D_L10_00161E9C SDATA(D_L10_00161E9C);
extern float D_L10_00161EA0 SDATA(D_L10_00161EA0);
extern int D_L10_00161EA4 SDATA(D_L10_00161EA4);
extern int D_L10_00161EA8 SDATA(D_L10_00161EA8);
extern int D_L10_00161EAC SDATA(D_L10_00161EAC);
extern int D_L10_00161EB0 SDATA(D_L10_00161EB0);
extern int D_L10_00161EB4 SDATA(D_L10_00161EB4);
extern int D_L10_00161EB8 SDATA(D_L10_00161EB8);
extern int D_L10_00161EBC SDATA(D_L10_00161EBC);
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern void func_L00_0025F8E0(float *v, float s);
extern float func_00213308(void);
extern float func_002132A8(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A68(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_001F9688(int);
extern int func_001FA6E0(int, int, float);
extern float func_001F96B0(float);
extern s32 func_001FA6D0(f32);
extern char *func_00218888(void *, void *, void *, int, int, int, int, int, int);

// Per-frame update of the moby in arg0: builds two vectors from its position and the trig helpers, then calls the level's helpers for each of its entries.
void FUN_L10_002e3a90(void *p) {
    char *arg0 = p;
    float v[4];
    float a[4];
    float b[4];
    float o[4];
    char *d = *(char **)(arg0 + 0x78);
    float f20;
    float f21;
    float f22;
    float r;
    int n = D_L10_00161E74;
    int i;
    int j;
    int k;
    int x16, x17, x18, x19, r7;
    for (i = 0; i < n; i++) {
        qcopy(v, arg0 + 0x10);
        func_L00_0025F8E0(v, D_L10_00161E7C);
        v[2] = v[2] + 0.55f;
        f22 = func_00213308();
        f20 = func_002132A8(D_L10_00161E88, D_L10_00161E8C);
        f21 = func_002132A8(D_L10_00161E90, D_L10_00161E94);
        if (((unsigned char)arg0[0x20]) == 0xB) {
            f20 = f20 * 0.55f;
            f21 = f21 * 0.55f;
            v[2] = v[2] - 0.2f;
        }
        r = func_001F9DC8(f22);
        a[0] = r * (f20 * D_0015ED6C);
        r = func_001F9DE0(f22);
        a[1] = r * (f20 * D_0015ED6C);
        a[2] = (f21 * D_0015ED6C) + 0.0f;
        func_001F9A68(o, d + 0x70, 0.5f);
        func_001F9A10(a, a, o);
        qcopy(b, a);
        k = func_001F9688(D_L10_00161EB8);
        b[2] = b[2] - (D_L10_00161E98 * D_0015ED70) * (float)k;
        f22 = 0.5f;
        f21 = 0.0f;
        f20 = 1.0f;
        for (j = 0; j < D_L10_00161E78; j++) {
            r = func_002132A8(f22, 1.5f);
            a[3] = r * D_L10_00161E9C;
            b[3] = r * D_L10_00161EA0;
            func_L00_0025F8E0(v, D_L10_00161E80);
            func_L00_0025F8E0(b, D_L10_00161E84 * D_0015ED6C);
            r = func_002132A8(f21, f20);
            x19 = func_001FA6E0(D_L10_00161EA4, D_L10_00161EAC, r);
            r = func_002132A8(f21, f20);
            func_001FA6E0(D_L10_00161EA8, D_L10_00161EB0, r);
            x18 = func_001F9688(D_L10_00161EB4);
            x17 = func_001F9688(D_L10_00161EB8);
            x16 = x17;
            r = func_002132A8((float)D_L10_00161EBC * 0.5f, (float)D_L10_00161EBC * 2.5f);
            r = func_001F96B0(r);
            r7 = func_001FA6D0(r);
            func_00218888(v, a, b, x19, x18, x17, x16, r7, -1);
        }
    }
}
