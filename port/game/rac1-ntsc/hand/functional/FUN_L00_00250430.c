/* Carried from the PAL decompilation's nonmatching/functional/func_L00_00251468.c (games/rac1) through the version
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
/* functional: medium. Returns (distance between a.xyz and b.xyz minus (a.w + b.w)) / 1024, as the VU0 code does (squared sum, sqrt, scale by 2^-10, which is exact); VU0 sqrt and sum rounding and denormal flushing can differ from C float in the last bit. */
float FUN_L00_00250430(float *a, float *b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    float d = sqrt(dx * dx + dy * dy + dz * dz);
    return (d - (a[3] + b[3])) * 0.0009765625f;
}
