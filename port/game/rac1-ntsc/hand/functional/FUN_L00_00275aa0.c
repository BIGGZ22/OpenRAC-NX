/* Carried from the PAL decompilation's nonmatching/shared/func_L00_00276940.c (games/rac1/pal) through the version
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
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern void func_001F9A10(void *, void *, void *);
extern unsigned char D_0013E533[] NOT_SDA;
extern int func_001F9770(void *);
extern void func_L00_00267A08(void *);

/* NON_MATCHING FUN_L00_00275aa0 -- src/overlays/shared/partupd_00272158.c
 * Best so far: SIZE ours 700 / retail 712, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Particle update (type 0x78): ramps size (m+0xC) and alpha over a short timer, adds a velocity vector to m+0x10
 *   Budget spent at 700/712 bytes (p7 is closest, structure and frame 0xA0 match). Remaining: retail copies the by
 *   also retail builds v+0xC into $7 then copies it to $s21 (ours computes it straight into the saved reg, and all
 */
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9AD8(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern float func_001F9AF0(void *);
extern short D_L00_0016024C;

// Particle update: ramps the size and alpha, moves by a velocity vector, steers it away from a nearby moby and kills the particle when its timer expires.
void FUN_L00_00275aa0(char *m) {
    char *v = m + 0x20;
    char *p;
    float f4;
    float f12;
    float a[4] __attribute__((aligned(16)));
    float d[4] __attribute__((aligned(16)));
    float e[4] __attribute__((aligned(16)));
    float *w;
    int t;
    m[8] += v[0xA];
    t = *(short *)(v + 8) - *(short *)(m + 0xA);
    if (t < 6) {
        int c;
        int c2;
        f4 = (float)t / 5.0f;
        *(float *)(m + 0xC) = *(float *)(v + 4) + ((*(float *)(m + 0x20) + *(float *)(v + 4)) * 0.5f - *(float *)(v + 4)) * f4;
        c = (unsigned char)v[0xB];
        c2 = (unsigned char)v[0xB] >> 1;
        f12 = (float)(c2 - c) * f4 + (float)c;
    } else {
        f4 = (float)*(short *)(m + 0xA) / func_001FA6C0(t - 6);
        *(float *)(m + 0xC) = *(float *)(m + 0x20) + ((*(float *)(m + 0x20) + *(float *)(v + 4)) * 0.5f - *(float *)(m + 0x20)) * f4;
        f12 = (float)((unsigned char)v[0xB] >> 1) * f4 + 0.0f;
    }
    *(int *)(m + 4) = (func_001FA6D0(f12) << 24) | (*(int *)(m + 4) & 0xFFFFFF);
    w = (float *)(v + 0xC);
    p = m + 0x10;
    a[0] = *(float *)(v + 0xC);
    a[1] = w[1];
    a[2] = w[2];
    *(int *)&a[3] = 0;
    func_001F9A10(p, p, a);
    if (*(int *)(v + 0x18) != 0) {
        func_001F9A28(d, *(char **)(v + 0x18) + 0x10, p);
        if (d[0] <= 2.0f && d[1] <= 2.0f) {
            func_001F9AD8(e, d, D_0013E533 + 0x10AD);
            func_L00_001FF500(e, e, *(float *)&D_L00_0016024C);
            func_001F9A10(p, p, e);
            if (d[0] <= 0.5f && d[1] <= 0.5f) {
                func_L00_001FF500(a, d, 0.03f);
            } else {
                func_L00_001FF500(a, d, func_001F9AF0(a));
            }
            *(float *)(v + 0xC) = a[0];
            w[1] = a[1];
            a[2] = a[2] + 0.02f;
            w[2] = a[2];
        }
    }
    if (func_001F9770(m + 0xA)) {
        func_L00_00267A08(m);
    }
}
