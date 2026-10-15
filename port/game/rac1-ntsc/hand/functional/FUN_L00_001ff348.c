/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FF2F8.c (games/rac1) through the version
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
/* functional: medium. Multiplies the xyz lanes of two VU0 vectors and writes the 16-byte result; w is copied from a. VU0 float multiply rounds and flushes denormals differently from the C FPU in the last bit, and the VU pipeline nop has no visible effect here. */
void FUN_L00_001ff348(float *dst, const float *a, const float *b) {
    float x = a[0] * b[0];
    float y = a[1] * b[1];
    float z = a[2] * b[2];
    float w = a[3];
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}
