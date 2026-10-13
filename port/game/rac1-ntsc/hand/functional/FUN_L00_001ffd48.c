/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FFCF8.c (games/rac1/pal) through the version
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
/* functional: medium. Multiplies the 16 floats at a1 by the float in f12 and stores them at a0, which the VU0 does with vmulx on four quadwords; VU float multiply rounding may differ from C's in the last bit. */
void FUN_L00_001ffd48(float *dst, float *src, float s) {
    int i;
    for (i = 0; i < 16; i++) {
        dst[i] = src[i] * s;
    }
}
