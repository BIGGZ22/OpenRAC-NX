/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002AE278.c (games/rac1) through the version
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
typedef int u128_c2acfd8 __attribute__((mode(TI)));

/* NON_MATCHING FUN_L00_002acfd8 -- src/overlays/shared/vendor_002AB910.c
 * Best so far: SIZE ours 1060 / retail 1076, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped after 5 runs (p0-p4; p3 closest). Moby steer: blends m+0x78 data with the level block at D_0013E533+0x
 *   Differences left: register numbering (retail pos=$19, m=$20, vel=$17, d=$18, g=$16, e=$30), the 0x12C join (re
 *   Would unblock: a source shape that gives retail's join order for the 0x208C==15 / 0x2FC test, and a way to ste
 */
extern char D_0013E533[];
extern char D_0013E05A[];
extern char D_0013A4E0[];
extern char D_L00_00173E60[];
extern float D_0015ED70 MACRO_ADDR;

extern int func_001F9688(int);
extern unsigned char *func_L00_0025C338(unsigned char *);
extern void func_001F9A10(void *, void *, void *);
extern float func_001F9B20(void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern float func_001F9AB0(void *, void *);
extern float func_001F9B80(void *, void *);
extern void func_001F9A68(void *, void *, float);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001FA6C0(int);
extern void func_L00_002C2488(float *, float *, float *, float, float, float);
extern int func_L00_001EFC70(void *, void *, int, int, int);

typedef union {
    u128_c2acfd8 q;
    float f[4];
} V2AE278_c2acfd8 __attribute__((aligned(16)));

/* Steers moby m toward pos and vel, blending its data at 0x78 with the level's global state. */
void FUN_L00_002acfd8(char *pos, char *m, char *vel) {
    char *d;
    char *g;
    char *e;
    char *p;
    char *p60;
    char *p64;
    V2AE278_c2acfd8 A, B, C, D, E;
    float f20, f21, t, z;
    short h;
    int ok;

    d = *(char **)(m + 0x78);
    *(short *)(d + 0x6A) = func_001F9688(0x1E);
    m[0x20] = 1;
    qcopy(d, vel);
    g = D_0013E533 + 0xE1D;
    p60 = *(char **)(d + 0x60);

    if (*(int *)(g + 0x208C) != 0xF
        && (*(int *)(g + 0x2FC) == 0 || func_L00_0025C338(*(unsigned char **)(g + 0x2FC)) == 0)) {
        e = D_0013E05A + 0x4C6;
        if (p60 != 0) {
            h = *(short *)(p60 + 0xA6);
            if (h == 0x56E || h == 0x58E) {
                func_001F9A10(d, d, d + 0x40);
            } else {
                p64 = *(char **)(d + 0x64);
                if (p64 != 0) {
                    f20 = func_001F9B20(vel);
                    func_001F9A28(&A, D_0013E533 + 0xE9D, p60 + 0x10);
                    func_L00_001FF500(&A, &A, 1.0f);
                    f21 = func_001F9AB0(&A, d + 0x40);
                    f20 = f21 + f20;
                    t = func_001F9B80(pos, p60 + 0x10) / f20;
                    if (0.0f < f21) {
                        t = t * f21;
                        func_001F9A68(&B, &A, t);
                    } else {
                        func_001F9A68(&B, d + 0x40, t);
                    }
                    func_001F9A10(&D, &B, p60 + 0x10);
                    B.q = D.q;
                    t = func_001FA688(func_L00_001FF8B0(*(float *)(g + 0x670), *(float *)(g + 0x674)),
                                      func_L00_001FF8B0(B.f[0] - *(float *)(g + 0x80), B.f[1] - *(float *)(g + 0x84)));
                    if (1.5707964f < t) {
                        f20 = func_001F9DC8(func_L00_001FF8B0(*(float *)(g + 0x670), *(float *)(g + 0x674)));
                        f20 = f20 * func_001F9B80(g + 0x80, &B);
                        f20 = f20 * 0.5f;
                        D.f[0] = f20;
                        f20 = func_001F9DE0(func_L00_001FF8B0(*(float *)(g + 0x670), *(float *)(g + 0x674)));
                        f20 = f20 * func_001F9B80(g + 0x80, &B);
                        f20 = f20 * 0.5f;
                        D.f[1] = f20;
                        *(int *)&D.f[2] = 0;
                        func_001F9A10(&E, &D, g + 0x80);
                        B.q = E.q;
                    }
                    f20 = D_0015ED70 * 11.0f;
                    if ((*(int *)(D_0013A4E0 + 0x2600) & 5) != 0) {
                        f21 = func_001FA6C0(*(unsigned char *)(e + 0xA));
                        z = f21 * 2.5f + 8.5f;
                    } else {
                        z = 8.5f;
                    }
                    func_L00_002C2488((float *)pos, B.f, C.f, f20, 1e-5f, z);
                    func_001F9A68(&C, &C, 0.65f);
                    func_001F9A68(d, d, 0.35f);
                    func_001F9A10(&D, d, &C);
                    *(u128_c2acfd8 *)d = D.q;
                }
            }
        }
    } else {
        func_001F9A10(d, d, g + 0x100);
    }
    *(short *)(d + 0x54) = func_001F9688(0x12C);
    qcopy(m + 0x10, pos);
    p = *(char **)(g + 0x2080);
    A.q = *(u128_c2acfd8 *)(p + 0x10);
    A.f[2] = *(float *)(pos + 8);
    ok = func_L00_001EFC70(&A, pos, 0, *(int *)(g + 0x2080), 0);
    if (ok) {
        m[0xBC] = 1;
        qcopy(m + 0x10, D_L00_00173E60);
    }
    if (*(unsigned char *)(e + 0xA)) {
        if (*(int *)(g + 0x2084) == 1) {
            z = *(float *)(d + 8);
            func_001F9A68(d, d, 1.5f);
            *(float *)(d + 8) = z / 1.5f;
        }
    }
}
