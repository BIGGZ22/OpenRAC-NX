/* Carried from the PAL decompilation's nonmatching/game/func_001F9CA0.c (games/rac1) through the version
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
 * The VU computes all three fields before the sqc2 stores them, so out may be a or b
 * (the level code crosses in place). The sqc2 also stores vf3.w, which the vopmsub does
 * not write: it is whatever vf3 held. The PAL build's identical instructions (fcheck,
 * func_001F9CA0) left 0 there on every captured call, and the callers read here use xyz only, so 0.
 */
void FUN_001f9ad8(float *out, float *a, float *b) {
    float acc0 = b[1] * a[2];
    float acc1 = b[2] * a[0];
    float acc2 = b[0] * a[1];
    float x = acc0 - a[1] * b[2];
    float y = acc1 - a[2] * b[0];
    float z = acc2 - a[0] * b[1];
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = 0.0f;
}
