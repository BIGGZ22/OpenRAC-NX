/* functional: high. Checked against the US assembly (lqc2, vmul.xyzw, sqc2): componentwise product, no change. */
/* Carried from the PAL decompilation's nonmatching/game/func_001F9C60.c (games/rac1) through the version
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
/* dst = a * b, componentwise on four floats (VU0 vmul.xyzw: load both, multiply, store). */
void FUN_001f9a98(float *dst, float *a, float *b) {
    float r0 = a[0] * b[0];
    float r1 = a[1] * b[1];
    float r2 = a[2] * b[2];
    float r3 = a[3] * b[3];
    dst[0] = r0;
    dst[1] = r1;
    dst[2] = r2;
    dst[3] = r3;
}
