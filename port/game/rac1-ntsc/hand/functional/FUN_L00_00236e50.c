/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002377E0.c (games/rac1) through the version
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
} HudElem_c236e50;
extern void func_L00_00235AD8(HudElem_c236e50 *, int *, int *, int, int);
extern float D_L00_0017E660[][4];

/* NON_MATCHING FUN_L00_00236e50 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 840 / retail 832, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_00236e50: draws a HUD element: sets e->w/h from globals, places it (00236400/00236468), draws the bac
 *   p5.c/p6.c is the best shape (same size 832, BYTES ~570): float math, call order and table loop are right (t re
 *   Remaining gaps: (1) loop.c strength-reduces `D + i*16` (arg to 1FF2F8) together with the float read, retail re
 *   (2) resulting saved-register order (retail: $19 float ptr, $20 i, $21 size, $22 tex, $23 v2, $30 hi) and early
 */
extern int func_001FF960(int, int);
extern void func_L00_0023BAC0(int, int, int, int, int, int);
extern void func_001F99F8(void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF348(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_00200080(int, int, int, int, int, int);
extern float D_L00_0017E664[];
extern short D_L00_0015F760;
extern short D_L00_0015F764;
extern short D_L00_0015F768;
extern short D_L00_0015F76C;
extern short D_L00_0015F750;
extern short D_L00_0015F754;
extern short D_L00_0015F758;

/* Draws a HUD element's backdrop and the 100 entries of its particle table that are due */
int FUN_L00_00236e50(HudElem_c236e50 *e) {
    float v0[4], v1[4], v2[4], v3[4], v4[4], v5[4];
    int x, y;
    float u, t;
    int tex, sz, i;

    x = e->unk50;
    y = e->unk54;
    e->w = *(int *)&D_L00_0015F760;
    e->h = *(int *)&D_L00_0015F764;
    func_L00_00235A70(e, &x, &y);
    func_L00_00235AD8(e, &x, &y, e->unk6C, 0);
    tex = func_001FF960(*(int *)e, 0);
    func_L00_0023BAC0(tex, x + e->unk48, y + e->unk4A, *(int *)&D_L00_0015F760, *(int *)&D_L00_0015F764, 0x80);
    u = (10000 - e->unk74) / 10000.0f;
    v0[0] = (*(int *)&D_L00_0015F760 * *(float *)&D_L00_0015F768 * 0.5f) / 17.0f * 16.0f;
    v0[1] = -(*(int *)&D_L00_0015F764 * *(float *)&D_L00_0015F76C * 0.5f) / 116.0f * 16.0f;
    v0[2] = 1.0f;
    v0[3] = 1.0f;
    v1[0] = (*(int *)&D_L00_0015F760 * *(float *)&D_L00_0015F768 * 0.5f + (x + e->unk48)
             + (1.0f - *(float *)&D_L00_0015F768) * 0.5f * *(int *)&D_L00_0015F760) * 16.0f;
    v1[1] = (*(int *)&D_L00_0015F764 * *(float *)&D_L00_0015F76C * 0.5f + (y + e->unk4A)
             + (1.0f - *(float *)&D_L00_0015F76C) * 0.5f * *(int *)&D_L00_0015F764) * 16.0f;
    v1[2] = 0.0f;
    v1[3] = 0.0f;
    t = 116.0f - (u + u) * 116.0f;
    tex = func_001FF960(*(int *)e, 1);
    func_001F99F8(v2);
    func_001F99F8(v3);
    func_001F99F8(v4);
    v2[0] = v2[1] = *(float *)&D_L00_0015F750 * 16.0f;
    v3[0] = v3[1] = (*(float *)&D_L00_0015F754 - *(float *)&D_L00_0015F750) * 16.0f;
    v4[0] = v4[1] = (*(float *)&D_L00_0015F758 - *(float *)&D_L00_0015F754) * 16.0f;
    sz = 0;
    for (i = 0; i < 100; i++) {
        if (i == 0) {
            func_001F9A28(v1, v1, v2);
            sz = (int)(*(float *)&D_L00_0015F750 * 32.0f);
            t += *(float *)&D_L00_0015F750;
        }
        if (t < D_L00_0017E664[i * 4]) continue;
        func_L00_001FF348(v5, D_L00_0017E660 + i * 16, v0);
        func_001F9A10(v5, v5, v1);
        func_00200080(tex, (int)v5[0], (int)v5[1], sz, sz, 0x80);
    }
    return e->w;
}
