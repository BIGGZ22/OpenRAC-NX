/* Carried from the PAL decompilation's nonmatching/shared/func_L14_002E1110.c (games/rac1) through the version
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

/* NON_MATCHING FUN_L14_002dfd10 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: SIZE ours 1128 / retail 1120, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Moby class 403 update on levels 14 and 15 (1120 bytes): two paths on m[0x20] (a copy of m+0x10 through 0025E86
 *   Differences left: retail keeps 1.0f in $f20 across the calls and saves $f21 at entry (ours folds the constants
 *   Unblock: a way to keep 1.0f in a callee-saved float register across calls without folding, and the retail poin
 */
extern int D_L14_0015F70C MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;
extern float D_0015ED64 MACRO_ADDR;
extern float D_L14_00161B48 SDATA(D_L14_00161B48);
extern float D_L14_00161B44 SDATA(D_L14_00161B44);
extern float D_L14_00161B40 SDATA(D_L14_00161B40);
extern int func_L00_0025D808(void *, void *, void *, void *, int, float);
extern void func_001F9A28(void *, void *, void *);
extern void func_L14_002E03B8(void *);
extern int func_001F9740(void *);
extern void func_L00_001FF500(void *, void *, float);
extern int func_L00_0028EF68_v(int, int, void *, int) __asm__("func_L00_0028DC90");
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern float AbsoluteFloat(float input) __asm__("func_001F99C0");
extern float func_001FA580(float, float);
extern int func_L00_001F0D60(float, void *, int, void *);
extern void func_0020C828(void *);
extern int func_00213260(int);

// Update function for moby class 403 on levels 14 and 15: steers the moby's floats and emits its effect.
void FUN_L14_002dfd10(char *m) {
    char *c;
    float vA[4];
    float vB[4];
    char *a1;
    int base;
    int r;
    int h;
    float f0;
    float f1;
    float f2;
    float f3;
    float f21;

    c = *(char **)(m + 0x78);
    if (*(unsigned char *)(m + 0x20) == 0) {
        base = D_L14_0015F70C + (*(int *)(c + 0xC) << 5);
        a1 = *(char **)(base + 0x10);
        qcopy(vA, m + 0x10);
        r = func_L00_0025D808(a1, m + 0x10, c + 0x10, c + 0x14, 0, *(float *)(c + 0x8));
        func_001F9A28(vB, m + 0x10, vA);
        *(float *)(c + 0x18) = vB[0];
        *(float *)(c + 0x1C) = vB[1];
        func_L14_002E03B8(m);
        if (func_001F9740(c + 4) == 0) {
            return;
        }
        f21 = 0.5f;
        func_L00_001FF500(vB, vB, f21);
        qcopy(vA, m + 0x10);
        vA[2] = vA[2] + 0.25f;
        h = *(short *)(*(char **)c + 0xA6);
        func_L00_0028EF68_v(1, 0, m, h);
        func_L00_0025E450(m, vB, vA, 0.0f, 0.0f, 5, 2, 4, 1.0f, 0.5f, 9.0f, 1.0f, -1, 15.0f, 0, 3, -1, 0);
        goto tail;
    }
    func_001F9A10(m + 0x10, m + 0x10, c + 0x30);
    f0 = AbsoluteFloat(*(float *)(c + 0x30));
    f3 = D_0015ED60;
    f1 = D_L14_00161B48 * f3;
    if (f1 < f0) {
        f2 = 1.0f;
        f0 = D_L14_00161B44;
        f1 = *(float *)(c + 0x30);
        f0 = (f0 - f2) * f3 + f2;
        *(float *)(c + 0x30) = f1 * f0;
    }
    f0 = AbsoluteFloat(*(float *)(c + 0x34));
    f3 = D_0015ED60;
    f1 = D_L14_00161B48 * f3;
    if (f1 < f0) {
        f2 = 1.0f;
        f0 = D_L14_00161B44;
        f1 = *(float *)(c + 0x34);
        f0 = (f0 - f2) * f3 + f2;
        *(float *)(c + 0x34) = f1 * f0;
    }
    f3 = *(float *)(c + 0x38);
    f21 = 0.0f;
    if (f21 < f3) {
        f2 = 1.0f;
        f0 = D_L14_00161B44;
        f1 = D_0015ED60;
        f0 = (f0 - f2) * f1 + f2;
        *(float *)(c + 0x38) = f3 * f0;
    }
    f2 = D_0015ED64;
    f0 = D_L14_00161B40 * f2;
    f1 = *(float *)(c + 0x38);
    f1 = f1 - f0;
    *(float *)(c + 0x38) = f1;
    *(float *)(m + 0x40) = func_001FA580(*(float *)(m + 0x40), *(float *)(c + 0x20));
    *(float *)(m + 0x44) = func_001FA580(*(float *)(m + 0x44), *(float *)(c + 0x24));
    *(float *)(m + 0x48) = func_001FA580(*(float *)(m + 0x48), *(float *)(c + 0x28));
    if (*(float *)(m + 0x10) < 2.0f || 1020.0f < *(float *)(m + 0x10) || *(float *)(m + 0x14) < 2.0f || 1020.0f < *(float *)(m + 0x14) || *(float *)(m + 0x18) < 2.0f || 1020.0f < *(float *)(m + 0x18)) {
        func_0020C828(m);
        return;
    }
    func_L00_001FF500(vB, m + 0xE0, 0.52f);
    func_001F9A10(vA, m + 0x10, vB);
    if (func_001F9740(c + 4) == 0) {
        r = func_L00_001F0D60(0.5f, vA, 0, (void *)*(int *)c);
        if (r == 0) {
            if (func_00213260(2) != 0) {
                func_L14_002E03B8(m);
            }
            return;
        }
    }
    h = *(short *)(*(char **)c + 0xA6);
    func_L00_0028EF68_v(0, 0, m, h);
    func_L00_0025E450(m, c + 0x30, m + 0x10, 0.0f, 0.0f, 5, 2, 4, 1.0f, 0.5f, 9.0f, 1.0f, -1, 15.0f, 0, 3, -1, 0);
tail:
    r = *(int *)(D_0013E533 + 0x2E9D);
    func_L00_001F0D60(1.0f, m + 0x10, 0, (void *)r);
    func_0020C828(m);
}
