/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002630A8.c (games/rac1/pal) through the version
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

/* NON_MATCHING FUN_L00_00262030 -- src/overlays/shared/mobyutil_00261B00.c
 * Best so far: BYTES 6/816 (99.3% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_00262030: clamps a point against a polyline (table D_L00_001B04B0[idx]: count at +0, 16-byte vertices
 *   p6.c (and p8.c) is BYTES 15/816: everything matches except the order of the argument-register moves at two cal
 *   Key shapes: j = i + 1 computed in the loop and `i = j` as the increment, index locals for the first tests, the
 *   fz3/z06: p9 (thr instead of th copy), p10 (r=1 after the 4B0 call), p11 (block-scoped float copies for the thr
 */
extern float func_001F9AB0(void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF4B0__s(float, void *, void *) __asm__("func_L00_001FF500");
extern void func_001F9AD8(void *, void *, void *);
extern float func_001F99C0(float);
extern float func_001F9AF0(void *);
extern void func_001F9A10(void *, void *, void *);
extern char D_L00_001B04B0[];

/* clamps a point to a polyline's segments and writes the pushed-out point */
int FUN_L00_00262030(int idx, void *pos, void *out, float thr) {
    float A[4], F[4], P[4], B[4], D[4], E[4], C[4];
    char *S;
    float base;
    int r = 0;
    int i, j, o;

    float th = thr;
    qcopy(P, pos);
    S = *(char **)(D_L00_001B04B0 + idx * 4);
    base = func_001F9AB0(P, D_0013E533 + 0x10AD);
    for (i = 0; i < *(int *)S - 1; i = j) {
        char *a;
        float d, len, t;
        j = i + 1;
        o = i * 16;
        if (*(float *)(S + o + 0x1C) == 0.0f && *(float *)(S - (-(j * 16)) + 0x1C) == 0.0f) continue;
        a = S + (o + 0x10);
        func_001F9A28(A, P, a);
        func_001F9A28(B, S + (o + 0x20), a);
        t = func_001F9AB0(A, D_0013E533 + 0x10AD);
        func_L00_001FF4B0__s(t, C, D_0013E533 + 0x10AD);
        func_001F9A28(A, A, C);
        t = func_001F9AB0(B, D_0013E533 + 0x10AD);
        func_L00_001FF4B0__s(t, C, D_0013E533 + 0x10AD);
        func_001F9A28(B, B, C);
        func_L00_001FF4B0__s(1.0f, D, B);
        func_001F9AD8(E, A, D);
        if (thr < func_001F99C0(func_001F9AB0(E, D_0013E533 + 0x10AD))) continue;
        len = func_001F9AF0(B);
        d = func_001F9AB0(A, D);
        if (len < d || d < 0.0f) {
            if (func_001F9AF0(A) < thr) {
                r = 1;
                func_L00_001FF4B0__s(th, F, A);
                func_001F9A10(P, F, a);
            }
        } else {
            r = 1;
            func_L00_001FF4B0__s(d, C, D);
            func_001F9A28(F, A, C);
            func_L00_001FF4B0__s(th, F, F);
            func_001F9A10(F, F, C);
            func_001F9A10(P, F, a);
        }
    }
    if (r != 0) {
        float f = base - func_001F9AB0(P, D_0013E533 + 0x10AD);
        func_L00_001FF4B0__s(f, B, D_0013E533 + 0x10AD);
        func_001F9A10(out, P, B);
    }
    return r;
}
