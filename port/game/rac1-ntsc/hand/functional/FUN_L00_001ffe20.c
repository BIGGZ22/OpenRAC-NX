/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FFDD0.c (games/rac1/pal) through the version
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
/* functional: medium. dst[0..15] = a[0..15] + b[0..15], element by element, as the four VU0 vadd.xyzw groups do; VU0 rounds the float sum in the last bit where C may differ. */
void FUN_L00_001ffe20(float *dst, float *a, float *b) {
    int i;
    for (i = 0; i < 16; i++) {
        dst[i] = a[i] + b[i];
    }
}
