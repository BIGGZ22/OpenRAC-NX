/* Carried from the PAL decompilation's nonmatching/functional/func_L01_00262D28.c (games/rac1/pal) through the version
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
/* functional: medium. Blend of two float buffers: dst = a*(1-f) + b*f over 23 iterations of 16 floats (368 floats), as the VU0 loop does per 16-byte vector. VU rounding is not reproduced. */
void FUN_L01_00261d78(float *dst, float *a, float *b, float f) {
    int i;
    for (i = 0; i < 23 * 16; i++) {
        dst[i] = a[i] * (1.0f - f) + b[i] * f;
    }
}
