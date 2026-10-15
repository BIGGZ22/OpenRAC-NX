/* Carried from the PAL decompilation's nonmatching/shared/func_L00_00236F38.c (games/rac1) through the version
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
typedef struct {
    char pad0[0x8];
    int unk08;
    int *unk0C;
    char pad10[0x38];
    short unk48;
    short unk4A;
    char pad4C[0x4];
    int unk50;
    int unk54;
    int w;
    int h;
    int flags;
    char pad64[0x8];
    int unk6C;
    unsigned char cnt[4];
    int unk74;
    int unk78;
    int unk7C;
    void *unk80;
} HudElem_c2365a8;

/* NON_MATCHING FUN_L00_002365a8 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 1380 / retail 1384, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   HUD update: sets rec+0x58/0x5C from two gp globals, calls 00236400/00236468, then draws up to 100 sprite pairs
 *   Still differing: which saved register holds each pointer and the flag order at the top, the add.s placement in
 *   Unblock: a source order that gets the 4 missing bytes (likely an extra store or branch shape in the i==0 case)
 */
extern int func_L00_00235A70(HudElem_c2365a8 *rec, int *x, int *y);
extern void func_L00_00235AD8(HudElem_c2365a8 *, int *, int *, int, int);
extern void func_00200E08(int, int, int, int, long, int);
extern int func_001F9688(int);
extern float func_001F96B0(float);
extern int func_L00_00216DE8(int, int);
extern int func_001FF960(int, int);
extern void func_001F99F8(float *);
extern void func_001F9A28(void *dst, void *a, void *b);
extern void func_L00_001FF348(float *, float *, float *);
extern void func_001F9A10(float *, float *, float *);
extern void func_00200080(int, int, int, int, int, int);
extern void func_L00_0023BAC0(int tex, int x, int y, int w, int h, int alpha);
extern int D_L00_0015F3F8 MACRO_ADDR;
extern float D_L00_0017E660[][4];
extern int D_L00_0015F760 SDATA(D_L00_0015F760);
extern int D_L00_0015F764 SDATA(D_L00_0015F764);
extern float D_L00_0015F768 SDATA(D_L00_0015F768);
extern float D_L00_0015F76C SDATA(D_L00_0015F76C);
extern short D_L00_0015F750;
extern short D_L00_0015F754;
extern short D_L00_0015F758;

// HUD element update: places sprite pairs along a 100-step arc
int FUN_L00_002365a8(HudElem_c2365a8 *rec) {
    char *r = (char *)rec;
    int xs, ys;
    int x, y, a, b, n, i, flagA, flagB, s, t21, tex, w;
    float v0[4], v10[4], v20[4], v30[4], v40[4], v50[4];
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f20, f21, f22, fA, fB;

    x = *(int *)(r + 0x50);
    y = *(int *)(r + 0x54);
    a = D_L00_0015F760;
    b = D_L00_0015F764;
    *(int *)(r + 0x58) = a;
    *(int *)(r + 0x5C) = b;
    func_L00_00235A70(rec, &x, &y);
    func_L00_00235AD8(rec, &x, &y, *(int *)(r + 0x6C), 0);

    a = D_L00_0015F760;
    b = D_L00_0015F764;
    {
        int X1 = (x + *(short *)(r + 0x48)) * 16 + (208 * a) / 64;
        int Y1 = (y + *(short *)(r + 0x4A)) * 16 + (160 * b) / 256;
        int X2 = X1 + (608 * a) / 64;
        int Y2 = Y1 + (3744 * b) / 256;
        func_00200E08(X1, Y1, X2, Y2, 0x80684C2F, 1);
    }

    n = *(int *)(r + 0x74);
    if (n * 3 < 10001) {
        s = func_001F9688(1);
        f0 = func_001F96B0(0.75f);
        flagA = (D_L00_0015F3F8 - 1) % (int)((float)s * 60.0f) < (int)(f0 * 60.0f);
    } else {
        flagA = 0;
    }
    n = *(int *)(r + 0x74);
    if (n * 3 < 10001) {
        s = func_001F9688(1);
        f0 = func_001F96B0(0.75f);
        flagB = D_L00_0015F3F8 % (int)((float)s * 60.0f) < (int)(f0 * 60.0f);
    } else {
        flagB = 0;
    }
    if (!flagA && flagB == 1) {
        func_L00_00216DE8(0xB, 0);
    }
    w = 0;
    tex = func_001FF960(*(int *)r, flagB);
    f20 = 16.0f;
    f22 = 32.0f;
    xs = x + *(short *)(r + 0x48);
    ys = y + *(short *)(r + 0x4A);
    func_L00_0023BAC0(tex, xs, ys, D_L00_0015F760, D_L00_0015F764, 0x80);

    fB = (float)D_L00_0015F764;
    fA = (float)D_L00_0015F760;
    n = *(int *)(r + 0x74);
    f4 = D_L00_0015F76C;
    f3 = fB * f4;
    f2 = 1.0f;
    f1 = D_L00_0015F768;
    f7 = (float)(10000 - n);
    f5 = 0.5f;
    f6 = f2 - f1;
    f1 = fA * f1;
    f0 = 10000.0f;
    f4 = f2 - f4;
    f3 = f3 * f5;
    f21 = f7 / f0;
    xs = x + *(short *)(r + 0x48);
    ys = y + *(short *)(r + 0x4A);
    f1 = f1 * f5;
    f4 = f4 * f5;
    f8 = 116.0f;
    f0 = (float)xs;
    f9 = 17.0f;
    f6 = f6 * f5;
    f7 = (float)ys;
    v0[2] = f2;
    v0[3] = f2;
    f5 = -f3;
    f0 = f1 + f0;
    v10[2] = 0.0f;
    v10[3] = 0.0f;
    f3 = f3 + f7;
    f6 = f6 * fA;
    f4 = f4 * fB;
    f1 = f1 / f9;
    f5 = f5 / f8;
    f0 = f0 + f6;
    f3 = f3 + f4;
    f2 = f21 + f21;
    f0 = f0 * f20;
    f3 = f3 * f20;
    f1 = f1 * f20;
    f2 = f2 * f8;
    v10[0] = f0;
    v10[1] = f3;
    v0[0] = f1;
    f5 = f5 * f20;
    v0[1] = f5;
    f21 = f8 - f2;
    t21 = func_001FF960(*(int *)r, 2);

    func_001F99F8(v20);
    func_001F99F8(v30);
    func_001F99F8(v40);
    f0 = *(float *)&D_L00_0015F750;
    f2 = *(float *)&D_L00_0015F754;
    f1 = *(float *)&D_L00_0015F758;
    f3 = f0 * f20;
    f0 = f2 - f0;
    f1 = f1 - f2;
    v20[0] = f3;
    v20[1] = f3;
    f0 = f0 * f20;
    v30[0] = f0;
    v30[1] = f0;
    f1 = f1 * f20;
    v40[0] = f1;
    v40[1] = f1;

    for (i = 0; i < 100; i++) {
        if (i == 0) {
            func_001F9A28(v10, v10, v20);
            f0 = *(float *)&D_L00_0015F750;
            f21 = f21 + f0;
            f1 = f0 * f22;
            w = (int)f1;
        } else if (i == 10) {
            func_001F9A28(v10, v10, v30);
            f0 = *(float *)&D_L00_0015F754;
            f1 = *(float *)&D_L00_0015F750;
            f2 = f0 * f22;
            f0 = f0 - f1;
            w = (int)f2;
            f21 = f21 + f0;
        } else if (i == 30) {
            func_001F9A28(v10, v10, v40);
            f0 = *(float *)&D_L00_0015F758;
            f1 = *(float *)&D_L00_0015F754;
            f2 = f0 * f22;
            f0 = f0 - f1;
            w = (int)f2;
            f21 = f21 + f0;
        }
        if (f21 < D_L00_0017E660[i][1]) continue;
        func_L00_001FF348(v50, D_L00_0017E660[i], v0);
        func_001F9A10(v50, v50, v10);
        func_00200080(t21, (int)v50[0], (int)v50[1], w, w, 0x80);
    }
    return *(int *)(r + 0x58);
}
