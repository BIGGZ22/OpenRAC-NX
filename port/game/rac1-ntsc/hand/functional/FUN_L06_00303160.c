/* Carried from the PAL decompilation's nonmatching/l06_blarg/func_L06_003045A0.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L06_00303160 -- src/overlays/l06_blarg/vendor_002FE5D0.c
 * Best so far: SIZE ours 1472 / retail 1512, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Blarg moby path update: three path points from the table (C, B, A), three func_001FA6D0 index lookups, a splin
 *   Runs 1-2: compile errors (global names). Run 3 (p0): compiles, SIZE 1472 vs 1512 (40 bytes short). Allocator k
 *   Runs 4-8 (p1 shared pointer locals, p2 reordered int locals, p3 nested if): all still SIZE 1472. Calls match r
 *   Left: size. The first byte difference is the saved-register set in the prologue (ours saves $s6 at the top, re
 */
extern int func_001F9688(int);
extern float func_00213ED8(float *, float, float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_001F9A40(void *, void *, void *, float);
extern void func_L00_002EA9D8(void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA5C8(float, float);
extern float func_001FA580(float, float);
extern f32 func_001F9D48_07408(union RegionVector *, union RegionVector *) __asm__("func_001F9B80");
extern float func_001F9DC8(float);
extern void func_001F99F8(void *);
extern void func_L00_002EAA30(void *);
extern float func_L00_0025BE00(float *, float *, float, float, float, float);
extern int func_0022DBA0(int, int, void *);
extern void func_L06_00303748(void *, void *, void *, int, float);
extern void func_L06_00303B68(void *);
extern int *D_L06_001B0C30[];
extern char *D_L06_001600EC MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;

/* Blarg moby path update: sets the moby's 0x2C, 0x44 and 0x48 float outputs from the path table. */
int FUN_L06_00303160(char *moby, int idx)
{
    char *data = *(char **)(moby + 0x78);
    char *q = data + (idx << 2);
    char *A = (char *)D_L06_001B0C30[*(int *)(q + 0x60)];
    char *B = (char *)D_L06_001B0C30[*(int *)(q + 0x74)];
    char *C = (char *)D_L06_001B0C30[*(int *)(q + 0x88)];
    float u[4];
    float w[4];
    float x, f20, f21, f23, t, r1, r2, r3, r4, t2;
    int i4, i5, i3, i2, i1;
    char *D;
    char *a1;
    char *c3;

    func_00213ED8((float *)(data + 0xF4), 1.0f, 1.0f / (float)func_001F9688(0x4B0));
    x = *(float *)(data + 0xF4) * (float)(*(int *)A - 1);
    i1 = func_001FA898_r(x);
    f20 = (float)i1;
    f21 = x - f20;
    i2 = func_001FA898_r(*(float *)(data + 0xF4) * (float)(*(int *)B - 1));
    x = *(float *)(data + 0xF4) * (float)(*(int *)C - 1);
    i3 = func_001FA898_r(x);
    f23 = x - f20;

    a1 = A + (i1 << 4);
    c3 = C + (i3 << 4);
    D = c3 + 0x20;
    func_001F9A40(u, a1 + 0x10, a1 + 0x20, f21);
    func_001F9A40(moby + 0x10, c3 + 0x10, D, f23);
    func_L00_002EA9D8(u);

    if (i3 > 0) {
        char *c2 = C + ((i3 - 1) << 4);
        char *c4 = C + ((i3 + 1) << 4);
        r1 = func_L00_001FF8B0(*(float *)(c3 + 0x10) - *(float *)(c2 + 0x10),
                               *(float *)(c3 + 0x14) - *(float *)(c2 + 0x14));
        f20 = r1;
        r2 = func_L00_001FF8B0(*(float *)(c4 + 0x10) - *(float *)(c3 + 0x10),
                               *(float *)(c4 + 0x14) - *(float *)(c3 + 0x14));
        *(float *)(moby + 0x48) = func_001FA580(func_001FA5C8(r2, r1) * f23, r1);
        t = func_001F9D48_07408((union RegionVector *)c3, (union RegionVector *)(c3 + 0x10));
        r3 = func_L00_001FF8B0(t, *(float *)(c3 + 0x18) - *(float *)(c2 + 0x18));
        f20 = -r3;
        t2 = func_001F9D48_07408((union RegionVector *)(c3 + 0x10), (union RegionVector *)D);
        r4 = func_L00_001FF8B0(t2, *(float *)(c4 + 0x18) - *(float *)(c3 + 0x18));
        *(float *)(moby + 0x44) = func_001FA580(func_001FA5C8(-r4, f20) * f23, f20);
        *(float *)(moby + 0x44) = *(float *)(moby + 0x44) * func_001F9DC8(*(float *)(data + 0xB0));
        *(float *)(moby + 0x48) = func_001FA580(*(float *)(moby + 0x48), *(float *)(data + 0xB0));
    } else {
        t = func_001F9D48_07408((union RegionVector *)(C + 0x10), (union RegionVector *)(C + 0x20));
        r1 = func_L00_001FF8B0(t, *(float *)(C + 0x28) - *(float *)(C + 0x18));
        *(float *)(moby + 0x44) = -r1;
        *(float *)(moby + 0x48) = func_L00_001FF8B0(*(float *)(C + 0x20) - *(float *)(C + 0x10),
                                                    *(float *)(C + 0x24) - *(float *)(C + 0x14));
    }

    if (i1 > 0) if (i2 > 0) {
        char *b2 = B + (i2 << 4);
        char *b3 = B + ((i2 + 1) << 4);
        func_001F99F8(w);
        r1 = func_L00_001FF8B0(*(float *)(b2 + 0x10) - u[0], *(float *)(b2 + 0x14) - u[1]);
        f20 = r1;
        r2 = func_L00_001FF8B0(*(float *)(b3 + 0x10) - u[0], *(float *)(b3 + 0x14) - u[1]);
        w[2] = func_001FA580(func_001FA5C8(r2, r1) * f23, r1);
        t = func_001F9D48_07408((union RegionVector *)u, (union RegionVector *)(b2 + 0x10));
        r3 = func_L00_001FF8B0(t, *(float *)(b2 + 0x18) - u[2]);
        f20 = -r3;
        t2 = func_001F9D48_07408((union RegionVector *)u, (union RegionVector *)(b2 + 0x20));
        r4 = func_L00_001FF8B0(t2, *(float *)(b3 + 0x18) - u[2]);
        w[1] = func_001FA580(func_001FA5C8(-r4, f20) * f23, f20);
        func_L00_002EAA30(w);
    }

    {
        float a = *(float *)(data + 0xC4);
        float bb = *(float *)(data + 0xC8);
        float f3 = *(float *)(data + 0xF4);
        float ff = a - (a - bb) * f3;
        *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * ff;
    }

    if (0.8f < *(float *)(data + 0xF4)) {
        func_L00_0025BE00((float *)(data + 0xB0), (float *)(data + 0xB4), 3.14159265f,
                          D_0015ED70 * 12.5663706f, D_0015ED70 * 12.5663706f, D_0015ED6C * 6.2831853f);
    }

    i4 = func_001FA898_r(*(float *)(A + (i1 << 4) + 0x1C));
    if (i1 == 0) {
        i5 = -1;
    } else {
        i5 = func_001FA898_r(*(float *)(A + ((i1 - 1) << 4) + 0x1C));
    }

    if (!(i4 < 0 && i5 < 0)) {
        *(float *)(A + (i1 << 4) + 0x1C) = -1.0f;
        *(float *)(A + ((i1 - 1) << 4) + 0x1C) = -1.0f;
        if (i4 < 8 && i5 < 8) {
            char *T2;
            func_0022DBA0(0, 0, moby);
            T2 = D_L06_001600EC + ((*(int *)(data + (i4 << 2) + 0xD0)) << 7);
            func_L06_00303748(moby, T2 + 0x30, T2, 6, 6.0f);
        } else if (i4 == 10 || i5 == 10) {
            func_0022DBA0(1, 0, moby);
            func_L06_00303B68(moby);
        }
    }

    return *(float *)(data + 0xF4) == 1.0f;
}
