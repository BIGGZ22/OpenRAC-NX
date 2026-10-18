/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FF610.c (games/rac1) through the version
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
/* functional: medium. Reflects a about the plane with normal b: n = b/|b|, t = -(a.n); when t is negative (sign bit set, as retail's bltz) dst is a copied whole, otherwise dst.xyz = a.xyz + 2*t*n and dst.w = a.w (joined with func_L00_001FF6C0, the copy). VU0 vrsqrt is approximate; this uses an exact sqrt. Checked against the US assembly 2026-10-17: a zero b gives n = 0 (retail's rsqrt of 0 is the largest float, not infinity), so no NaN. */
void FUN_L00_001ff660(float *dst, float *a, float *b) {
    float len2 = b[0] * b[0] + b[1] * b[1] + b[2] * b[2];
    float inv = len2 > 0.0f ? 1.0f / sqrtf(len2) : 0.0f;
    float nx;
    float ny;
    float nz;
    float t;
    if (inv > 3.4028235e38f) {
        inv = 3.4028235e38f;
    }
    nx = b[0] * inv;
    ny = b[1] * inv;
    nz = b[2] * inv;
    t = -(a[0] * nx + a[1] * ny + a[2] * nz);
    if (*(int *)&t < 0) { /* the sign bit, as the EE's branch on the word tests it */
        dst[0] = a[0];
        dst[1] = a[1];
        dst[2] = a[2];
        dst[3] = a[3];
        return;
    }
    dst[0] = a[0] + 2 * t * nx;
    dst[1] = a[1] + 2 * t * ny;
    dst[2] = a[2] + 2 * t * nz;
    dst[3] = a[3];
}
