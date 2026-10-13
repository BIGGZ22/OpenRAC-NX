/* Carried from the PAL decompilation's nonmatching/game/func_001F9CA0.c (games/rac1/pal) through the version
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
/*
 * FUN_001f9ad8(out, a, b): out = b x a, from the xyz of the two vectors (VU0
 * vopmula / vopmsub: ACC = b.yzx * a.zxy, then out = ACC - a.yzx * b.zxy).
 * The VU writes only x, y and z of the result; the sqc2 also stores vf3.w,
 * which is whatever the VU0 register held (left as it is here).
 */
void FUN_001f9ad8(float *out, float *a, float *b) {
    float acc0 = b[1] * a[2];
    float acc1 = b[2] * a[0];
    float acc2 = b[0] * a[1];
    out[0] = acc0 - a[1] * b[2];
    out[1] = acc1 - a[2] * b[0];
    out[2] = acc2 - a[0] * b[1];
}
