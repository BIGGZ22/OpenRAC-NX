/* Carried from the PAL decompilation's nonmatching/functional/func_L08_00259040.c (games/rac1) through the version
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
/* functional: high. Exact logic of handwritten PS2 VU0 UV shift assembly FUN_L08_00258090.
 * Offsets count 2D UV float pairs from src to dst by (offset[0], offset[1]).
 */
void FUN_L08_00258090(float *dst, const float *src, int count, const float *offset) {
    float du = offset[0];
    float dv = offset[1];
    int i;

    for (i = 0; i < count; i++) {
        dst[i * 2 + 0] = src[i * 2 + 0] + du;
        dst[i * 2 + 1] = src[i * 2 + 1] + dv;
    }
}
