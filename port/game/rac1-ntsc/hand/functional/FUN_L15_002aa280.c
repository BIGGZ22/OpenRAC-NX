/* Carried from the PAL decompilation's nonmatching/l15_quartu/func_L15_002AB4A8.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L15_002aa280 -- src/overlays/l15_quartu/vendor_0029C1D0.c
 * Best so far: SIZE ours 1788 / retail 1736, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 15 lamp-style moby update (1736 bytes): finds the matching entry in D_L15_00178700, then runs two 24/20-
 *   Wall for the next worker: the search and the copies both need matching; the loop bodies were transcribed from 
 */
extern char D_L15_00178700[];
extern char D_L15_001615B8[];
extern char D_L15_001615C8[];
extern unsigned char D_0013E533[];
extern char D_0013DD4D[];
extern unsigned char D_0014161B[] NOT_SDA;
extern float D_0015ED6C MACRO_ADDR;
extern int D_0015ED84 MACRO_ADDR;
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A68(void *, void *, float);
extern float func_001F9AF0(void *);
extern float func_002132A8(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_L00_001FF290(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern int func_00213260(int);
extern int func_L00_00257B90(int, int);
extern void func_L01_002F8530(void *, void *, unsigned int, int, float, float, float, float, int);
extern void func_001F99F8(void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_002371E0(int, int, float);
extern int func_001F9688(int);
extern char *func_L00_0026BED0(float f, char *a, char *b, int c, int d, int e, int g);
extern void func_L00_00257470(void *, int, int);
extern void func_0020C828(void *);

/* Level 15 moby update: aims the effect sprites at the moby, then deletes it when its flags say so. */
void FUN_L15_002aa280(unsigned char *m) {
    char *base = D_L15_00178700;
    char *found = 0;
    char *e;
    int i, n, w, r16, r17, r18;
    float f0, f20, f21, f22, f23, f24, f25, f26;
    float v20[4], v30[4], v40[4], v50[4], v80[4], v90[4];
    char v60[16], v70[16];
    void *tab[3];
    unsigned short v;

    if (*(unsigned char *)(D_0013E533 + 0x2EC1) != 2 && m[0x20] == 1) return;
    if (*(unsigned char **)(base + 0x34) == m && (*(int *)(base + 0x24) & 0x80000)) {
        found = base;
    } else {
        for (i = 1; i < 0x40; i++) {
            e = base + i * 64;
            if (*(unsigned char **)(e + 0x34) == m && (*(int *)(e + 0x24) & 0x80000)) {
                found = e;
                break;
            }
        }
    }
    m[0xA4] = 0xFF;
    if (found) {
        qcopy(v70, D_L15_001615B8);
        f0 = D_0015ED6C;
        func_L00_001FF500(v30, found + 0x10, D_0015ED6C * 10.0f);
        func_001F9A68(v40, v30, 2.0f);
        f0 = func_001F9AF0(found);
        if (f0 == 0.0f) {
            qcopy(v60, m + 0x10);
            *(float *)(v60 + 8) = *(float *)(v60 + 8) + 6.0f;
        } else {
            qcopy(v60, found);
        }
        tab[0] = v50;
        tab[1] = v80;
        tab[2] = v90;
        f24 = 1.5707964f;
        f25 = -3.0f;
        f21 = 3.0f;
        f22 = 7.0f;
        i = 0x18;
        do {
            i--;
            f23 = 5.0f;
            f0 = func_002132A8(*(float *)(m + 0x48), f24);
            f20 = func_001F9DC8(f0);
            f0 = func_002132A8(f25, f21);
            f20 = f20 * f0;
            v50[0] = f20;
            f0 = func_002132A8(*(float *)(m + 0x48), f24);
            f20 = func_001F9DE0(f0);
            f0 = func_002132A8(f25, f21);
            f20 = f20 * f0;
            v50[1] = f20;
            f0 = func_002132A8(1.0f, 7.0f);
            v50[2] = f0;
            func_L00_001FF290(v80, v50, m + 0x10);
            func_001F9A28(v80, v60, v50);
            qcopy(v20, v80);
            func_L00_001FF500(v20, v20, D_0015ED6C * 5.0f);
            func_L00_001FF290(v80, v20, v40);
            f0 = D_0015ED6C;
            v20[2] = v20[2] + f0 * 3.0f;
            f0 = func_002132A8(0.75f, 1.25f);
            func_001F9A68(v20, v20, f0);
            r16 = func_00213260(4) * 4;
            n = r16;
            f20 = func_002132A8(0.1f, 0.3f);
            r17 = func_L00_00257B90(100, 160);
            f0 = func_002132A8(f21, f22);
            func_L01_002F8530(v50, v20, *(unsigned int *)(v70 + n), r17, f20, f0, 0.15f, 2.0f, 1);
        } while (i >= 0);
        f22 = 1.0f;
        func_001F99F8(v20);
        f26 = 0.5f;
        func_L00_0025E450(m, v20, v60, 0.0f, 0.0f, 20, 3, 4, 10.0f, 5.0f, 100000.0f, 0, 3.0f, 15.0f, 1, 1, -1, 0);
        f23 = -3.0f;
        i = 0x13;
        do {
            qcopy(v80, D_L15_001615C8);
            f0 = func_002132A8(*(float *)(m + 0x48), f24);
            f20 = func_001F9DC8(f0);
            f0 = func_002132A8(f23, f21);
            f20 = f20 * f0;
            v50[0] = f20;
            f0 = func_002132A8(*(float *)(m + 0x48), f24);
            func_001F9DE0(f0);
            f0 = func_002132A8(f23, f21);
            f20 = f0 * f0;
            func_001F9A10(v50, v50, m + 0x10);
            v50[1] = f20;
            f0 = func_002132A8(f22, 5.5f);
            v50[2] = v50[2] + f0;
            func_001F9A28(v80, v60, v50);
            qcopy(v20, v90);
            func_L00_001FF500(v20, v20, D_0015ED6C * 5.0f);
            func_L00_001FF290(v90, v20, v30);
            f0 = D_0015ED6C;
            v20[2] = v20[2] + f0 * 3.0f;
            f0 = func_002132A8(0.75f, 1.25f);
            func_001F9A68(v20, v20, f0);
            f0 = func_002132A8(0.25f, f22);
            r18 = func_L00_002371E0(0x7F000000, 0x7F182030, f0);
            f0 = func_002132A8(f26, f22);
            r17 = func_L00_002371E0(0, 0x5F5F5F, f0);
            f0 = func_002132A8(f22, f21);
            r16 = func_001F9688(0xB4);
            f20 = f0 * 220000.0f;
            r16 = func_00213260(3) * 4;
            n = *(int *)((char *)tab[1] + r16);
            r16 = (int)func_L00_0026BED0(f20, (char *)v50, (char *)v20, r18, r17, r16, n);
            i--;
            if (r16) *(float *)(r16 + 0x2C) = *(float *)(m + 0x18) - f26;
        } while (i >= 0);
        func_L00_00257470(m, 0, -1);
        func_0020C828(m);
        return;
    }
    if (m[0x20] != 0) return;
    m[0x20] = 1;
    *(short *)(m + 0x32) = 0x80;
    m[0x30] = 0xFF;
    if (*(unsigned char *)(D_0013DD4D + 3) == 0) return;
    v = *(unsigned short *)(m + 0xB2);
    w = *(int *)((char *)D_0014161B + 0xAB75 + ((((short)v) >> 5) * 4) + D_0015ED84 * 256);
    if ((w >> (v & 0x1F)) & 1) func_0020C828(m);
}
