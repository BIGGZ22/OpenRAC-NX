/* Carried from the PAL decompilation's nonmatching/l08_batalia/func_L08_002D68D0.c (games/rac1) through the version
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
extern int func_0022DA68(int,int,void*);

/* NON_MATCHING FUN_L08_002d55c8 -- src/overlays/l08_batalia/vendor_002B9438.c
 * Best so far: SIZE ours 892 / retail 900, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped at budget: best p4.c (892 bytes against 900; p9.c is the same size). Steers the moby toward a route en
 */
extern int D_L08_001B0CB0[];
extern char D_0013E533[];
extern short D_L08_001618A0;
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;

extern float func_L00_001FF8B0(float, float);
extern float func_L00_0025BE00(float *, float *, float, float, float, float);
extern float func_00213ED8(float *, float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern float func_00213508(void *, int, float);
extern float func_001F9B80(void *, void *);
extern float func_001F9AF0(void *a);
extern int func_L00_0028D8C0(void *, int);
extern int func_L00_0028DF38(int, int);
extern int func_001F9688(int);
extern void func_L00_0028D918(int);

/* Steers the moby along the current route entry and keeps its two tracked slots in step. */
void FUN_L08_002d55c8(char *moby, int idx) {
    char *data = *(char **)(moby + 0x78);
    char *route = (char *)D_L08_001B0CB0[idx];
    float vec[4] __attribute__((aligned(16)));
    float w[4];
    float f2;
    float t;
    int r;
    int r2;
    char *e;
    char *p;
    char *m10 = moby + 0x10;
    char *m48 = moby + 0x48;

    qcopy(vec, route + 0x10 + (*(int *)(data + 0x168) << 4));
    f2 = func_L00_001FF8B0(vec[0] - *(float *)m10, vec[1] - *(float *)(moby + 0x14));
    func_L00_0025BE00((float *)m48, (float *)(data + 0x158), f2,
                      D_0015ED70 * 4.71238899f, D_0015ED70 * 4.71238899f, D_0015ED6C * 3.14159274f);
    func_00213ED8((float *)(data + 0x180), *(float *)(data + 0x16C), D_0015ED70 + D_0015ED70);
    w[0] = func_001F9DC8(*(float *)m48) * *(float *)(data + 0x180);
    w[1] = func_001F9DE0(*(float *)m48) * *(float *)(data + 0x180);
    w[2] = 0.0f;
    func_001F9A10(m10, m10, w);
    *(float *)(moby + 0x18) = func_00213508(m10, 0, 0.5f);
    if (func_001F9B80(m10, w) < 0.5f) {
        *(int *)(data + 0x168) = (*(int *)(data + 0x168) + 1) % *(int *)route;
    }
    f2 = *(float *)(data + 0x180) / (*(float *)&D_L08_001618A0 * D_0015ED6C);
    if (2.0f < f2) {
        f2 = 2.0f;
    } else if (f2 < 0.0f) {
        f2 = 0.0f;
    }
    if ((p = *(char **)(data + 0x160)) != 0) {
        *(float *)(p + 0x58) = f2;
        qcopy(*(char **)(data + 0x160) + 0x40, m48);
        qcopy(*(char **)(data + 0x160) + 0x10, m10);
    }
    if ((p = *(char **)(data + 0x164)) != 0) {
        *(float *)(p + 0x58) = f2;
        qcopy(*(char **)(data + 0x164) + 0x40, m48);
        qcopy(*(char **)(data + 0x164) + 0x10, m10);
    }
    t = D_0015ED6C * 0.1f;
    if (t < func_001F9AF0(data + 0x40)) {
        r = func_L00_0028D8C0(moby, *(int *)(data + 0x15C));
        if (r == 0) {
            *(int *)(data + 0x15C) = func_0022DA68(1, 4, (int)moby);
            func_L00_0028DF38(*(int *)(data + 0x15C), 1);
        } else {
            e = D_0013E533 + 0x1D + *(int *)(data + 0x15C) * 0x70;
            if (*(int *)(e + 0x80) < 0x400) {
                r2 = func_001F9688(0x3C);
                func_L00_0028DF38(*(int *)(data + 0x15C), *(int *)(e + 0x80) + 0x400 / r2);
            }
        }
    } else {
        r = func_L00_0028D8C0(moby, *(int *)(data + 0x15C));
        if (r != 0) {
            e = D_0013E533 + 0x1D + *(int *)(data + 0x15C) * 0x70;
            if (*(int *)(e + 0x80) < 0x100) {
                func_L00_0028D918(*(int *)(data + 0x15C));
                *(int *)(data + 0x15C) = -1;
            } else {
                r2 = func_001F9688(0x3C);
                func_L00_0028DF38(*(int *)(data + 0x15C), *(int *)(e + 0x80) - 0x400 / r2);
            }
        }
    }
}
