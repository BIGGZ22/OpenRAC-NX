/* Carried from the PAL decompilation's nonmatching/l18_veldin2/func_L18_002D96B0.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L18_002d82c0 -- src/overlays/l18_veldin2/vendor_002A8400.c
 * Best so far: SIZE ours 1128 / retail 1104, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Floating platform update (moby class 587, 1104 bytes): calls func_001F9A68 to negate the moby offset into a st
 *   Best candidate p3.c: 1128 bytes (retail 1104). The jump table is right with a case label for every state (stat
 *   Unblock: a way to pin the 1.0f register load ahead of the multiplies (no new flags), or a regalloc dump for th
 */
extern void func_001F9A68(void *, void *, float);
extern float func_00213308(void);
extern int func_001F9740(int *);
extern float func_001FA6C0(int);
extern void func_L18_002D88B8(char *moby);
extern void func_L18_002D8710(char *moby);
extern int func_L00_00257B90(int, int);
extern int func_001F9688(int);
extern int func_0022ED80_2F0F78(int, int, int) __asm__("func_0022DA68");
extern float func_001F9B80(void *, void *);
extern void func_L18_00227DD0(int, int);
extern void func_0020C828(void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_00260738(char *, void *, void *, void *);
extern char D_0013E533[];
extern short D_L18_001623A8;
extern short D_L18_00161A60;
extern short D_L18_00161A64;
extern short D_L18_00161A50;
extern short D_L18_00161A58;
extern short D_L18_00161A5C;
extern short D_L18_00161A40;
extern short D_L18_00161A44;
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern float D_L18_0015F638 MACRO_ADDR;

/* Floating platform update (moby class 587): eases its height and tilt towards the sway target by state. */
void FUN_L18_002d82c0(char *m) {
    char *data;
    char *p;
    float v0[4] __attribute__((aligned(16)));
    float v10[4] __attribute__((aligned(16)));
    float a;
    float b;
    float t;
    float u;
    float k;
    int r;
    float rf;
    float f;

    func_001F9A68(v0, m + 0x10, -1.0f);
    data = *(char **)(m + 0x78);
    qcopy(v10, m + 0x40);
    switch (*(unsigned char *)(m + 0x20)) {
    case 0:
        f = *(float *)(m + 0x18);
        *(float *)(data + 0x60) = f;
        *(int *)(data + 0x70) = 0;
        *(int *)(data + 0x68) = 0;
        *(int *)(data + 0x6C) = 0;
        *(float *)(data + 0x64) = f;
        rf = func_00213308();
        *(int *)(data + 0x5C) = *(int *)(data + 0x5C) | 1;
        *(float *)(data + 0x74) = rf;
        if (*(int *)(data + 0x88) != 0) {
            m[0x20] = 2;
        } else {
            m[0x20] = 1;
        }
        break;
    case 3:
        if (func_001F9740((int *)(data + 0x7C)) == 0) {
            break;
        }
        a = func_001FA6C0(*(int *)&D_L18_001623A8) - *(float *)&D_L18_00161A60;
        b = *(float *)&D_L18_00161A64 - *(float *)&D_L18_00161A60;
        t = a / b;
        if (1.0f < t) {
            t = 1.0f;
        } else if (t < 0.0f) {
            t = 0.0f;
        }
        u = t * 0.333f;
        if (*(int *)(data + 0x84) != 0) {
            k = *(float *)&D_L18_00161A50;
        } else {
            k = *(float *)&D_L18_00161A58;
        }
        *(float *)(data + 0x6C) = (k * (1.0f - u)) * 0.017453292f * D_0015ED6C;
        m[0x20] = 4;
        break;
    case 4:
        if (*(unsigned char *)(m + 0x31) != 0 && func_001F9740((int *)(data + 0x8C)) != 0) {
            func_L18_002D88B8(m);
            r = func_L00_00257B90(0x5A, 0x78);
            *(int *)(data + 0x8C) = func_001F9688(r);
        }
        p = D_0013E533 + 0xE1D;
        if (*(char **)(p + 0x2FC) == m && *(short *)(p + 0x30E) == 0) {
            *(int *)(data + 0x8C) = 0;
            m[0x20] = 5;
            *(short *)(data + 0x92) = func_0022ED80_2F0F78(1, 0, (int)m);
        }
        if (*(int *)(data + 0x80) == 0) {
            break;
        }
        if (func_001F9B80(m + 0x10, D_0013E533 + 0xE9D) < *(float *)&D_L18_00161A5C) {
            m[0x20] = 5;
            *(short *)(data + 0x92) = func_0022ED80_2F0F78(1, 0, (int)m);
        }
        break;
    case 5:
        if (*(unsigned char *)(m + 0x31) != 0 && func_001F9740((int *)(data + 0x8C)) != 0) {
            func_L18_002D88B8(m);
            r = func_L00_00257B90(0xF, 0x1E);
            *(int *)(data + 0x8C) = func_001F9688(r);
        }
        a = func_001FA6C0(*(int *)&D_L18_001623A8) - *(float *)&D_L18_00161A60;
        b = *(float *)&D_L18_00161A64 - *(float *)&D_L18_00161A60;
        t = a / b;
        if (1.0f < t) {
            t = 1.0f;
        } else if (t < 0.0f) {
            t = 0.0f;
        }
        u = t * 0.333f;
        if (*(int *)(data + 0x80) != 0 || (*(int *)(data + 0x88) != 0 && *(int *)(data + 0x84) == 0)) {
            *(float *)(data + 0x70) = *(float *)(data + 0x70) - D_0015ED70 * 15.0f;
        } else if (*(int *)(data + 0x84) != 0) {
            *(float *)(data + 0x70) = *(float *)(data + 0x70) - *(float *)&D_L18_00161A40 * (1.0f - u) * D_0015ED70;
        } else {
            *(float *)(data + 0x70) = *(float *)(data + 0x70) - *(float *)&D_L18_00161A44 * (1.0f - u) * D_0015ED70;
        }
        *(float *)(data + 0x64) = *(float *)(data + 0x64) + *(float *)(data + 0x70);
        if (*(float *)(m + 0x18) < *(float *)(data + 0x60) - 10.0f) {
            p = D_0013E533 + 0xE1D;
            if (*(char **)(p + 0x2FC) == m && *(short *)(p + 0x30E) == 0) {
                if (*(float *)(p + 0x88) < D_L18_0015F638) {
                    if (*(int *)(p + 0x2084) != 0x77) {
                        func_L18_00227DD0(0x77, 1);
                    }
                }
            }
            *(float *)(data + 0x70) = *(float *)(data + 0x70) - D_0015ED70 * 13.7f;
            if (*(float *)(m + 0x18) < 15.0f) {
                func_0020C828(m);
                return;
            }
        }
        break;
    case 1:
        break;
    case 2:
        break;
    default:
        break;
    }
    func_L18_002D8710(m);
    func_001F9A10(v0, v0, m + 0x10);
    func_L00_00260738(data + 0x20, v0, v10, m + 0x40);
}
