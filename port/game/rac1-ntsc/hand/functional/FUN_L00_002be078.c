/* Carried from the PAL decompilation's src/overlays/shared/vendor_002BA7C8.c (matched C there) (games/rac1) through the version
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
extern void func_001F9A28(void *, void *, void *);
extern float D_L00_00166DC0[];
extern void func_001F9AD8(void *, void *, void *);
extern float func_001F9AF0(void *);
extern void func_001F9A68(void *, void *, float);
extern short D_L00_00161774;
extern short D_L00_00161778;
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FD228(void *, void *, int);
extern void func_00233980(int, long);
typedef struct { float v[4]; } P_2BF380_c2be078;
typedef float RW_2BF380_c2be078[4] __attribute__((aligned(16)));
typedef struct {
    int c[4];
    float f[8];
    long z4, q0, q1, q2;
} RT_2BF380_c2be078;
extern int func_001F44B8(int);
extern short D_L00_001617A4;
extern short D_L00_00161780;
extern short D_L00_00161780;
extern short D_L00_001617A8;
extern short D_L00_001617A8;
typedef float RW_2BF380_c2be078[4] __attribute__((aligned(16)));
extern short D_L00_0016178C;
extern short D_L00_00161790;

void FUN_L00_002be078(P_2BF380_c2be078 *m, int a, int b, float sx, float sy, float sz, int n, int col) {
    RW_2BF380_c2be078 q[4];
    RT_2BF380_c2be078 t;
    RW_2BF380_c2be078 q2[4];
    RT_2BF380_c2be078 u;
    RW_2BF380_c2be078 w120, w130, w140, w150, r160, r170, a180, b190, s1A0, s1B0;
    int i, col0, col1;
    float len, len0, len2, l0, l1;
    t.q0 = func_001F44B8(*(int *)&D_L00_001617A4);
    col0 = *(int *)&D_L00_00161780 | (a << 24);
    t.q2 = 0x8000000048L;
    t.q1 = 0xFF9000000260L;
    t.z4 = 0;
    t.c[3] = col0;
    t.c[2] = col0;
    t.c[1] = col0;
    t.c[0] = col0;
    u.q0 = func_001F44B8(*(int *)&D_L00_001617A8);
    col1 = col | (b << 24);
    u.q2 = t.q2;
    u.q1 = t.q1;
    u.z4 = t.z4;
    u.c[3] = col1;
    u.c[2] = col1;
    u.c[1] = col1;
    u.c[0] = col1;
    u.f[0] = 0; u.f[1] = 0; u.f[2] = 0; u.f[3] = 1.0f; u.f[4] = 1.0f; u.f[5] = 0; u.f[6] = 1.0f; u.f[7] = 1.0f;
    t.f[0] = u.f[0] + *(float *)&D_L00_0016178C;
    t.f[1] = u.f[1];
    t.f[2] = u.f[2] + *(float *)&D_L00_0016178C;
    t.f[3] = u.f[3];
    t.f[4] = u.f[4] + *(float *)&D_L00_0016178C;
    t.f[5] = u.f[5];
    t.f[6] = u.f[6] + *(float *)&D_L00_0016178C;
    t.f[7] = u.f[7];
    func_001F9A28(w140, m, D_L00_00166DC0);
    func_001F9A28(w150, &m[1], m);
    func_001F9AD8(w120, w150, w140);
    len0 = func_001F9AF0(w120);
    if (len0 != 0) len0 = 1.0f / len0;
    func_001F9A68(w120, w120, *(float *)&D_L00_00161774 * len0);
    func_001F9A10(q[0], m, w120);
    func_001F9A28(q[1], m, w120);
    for (i = 1; i < n - 1; i++) {
        func_001F9A28(w140, &m[i], D_L00_00166DC0);
        func_001F9A28(w150, &m[i + 1], &m[i]);
        func_001F9AD8(w120, w150, w140);
        len = func_001F9AF0(w120);
        if (len != 0) len = 1.0f / len;
        func_001F9A68(w120, w120, *(float *)&D_L00_00161774 * len);
        func_001F9A10(q[2], &m[i], w120);
        func_001F9A28(q[3], &m[i], w120);
        if (i == 1) {
            t.c[1] = *(int *)&D_L00_00161780;
            t.c[0] = *(int *)&D_L00_00161780;
        } else if (i == n - 2) {
            t.c[3] = *(int *)&D_L00_00161780;
            t.c[2] = *(int *)&D_L00_00161780;
        } else if (i == 2) {
            t.c[1] = col0;
            t.c[0] = col0;
        }
        func_L00_001FD228(q, 0, 0);
        qcopy(q[0], q[2]);
        qcopy(q[1], q[3]);
    }
    for (i = 0; i < n - 1; i++) {
        func_001F9A28(a180, &m[i], D_L00_00166DC0);
        func_001F9A28(b190, &m[i + 1], D_L00_00166DC0);
        l0 = func_001F9AF0(a180);
        l1 = func_001F9AF0(b190);
        if (l1 < l0) {
            if (l1 == 0) qcopy(b190, a180);
            else func_001F9A68(b190, b190, l0 / l1);
        } else {
            if (l0 == 0) qcopy(a180, b190);
            else func_001F9A68(a180, a180, l1 / l0);
        }
        func_001F9A10(r160, D_L00_00166DC0, a180);
        func_001F9A10(r170, D_L00_00166DC0, b190);
        func_001F9A28(w150, r170, r160);
        func_001F9A68(s1B0, w150, sz);
        func_001F9A28(r160, r160, s1B0);
        func_001F9A10(r170, r170, s1B0);
        func_001F9A28(s1A0, D_L00_00166DC0, r160);
        func_001F9AD8(w120, s1A0, w150);
        len2 = func_001F9AF0(w120);
        if (len2 != 0) len2 = 1.0f / len2;
        func_001F9A68(w130, w120, sy * len2);
        func_001F9A10(q2[0], r160, w130);
        func_001F9A28(q2[1], r160, w130);
        func_001F9A10(q2[2], r170, w130);
        func_001F9A28(q2[3], r170, w130);
        if (i == 0) {
            u.c[1] = col;
            u.c[0] = col;
        } else if (i == n - 2) {
            u.c[3] = col;
            u.c[2] = col;
        } else if (i == 1) {
            u.c[1] = col1;
            u.c[0] = col1;
        }
        func_L00_001FD228(q2, 0, 0);
    }
    func_00233980(0x47, 0x5360B);
}
