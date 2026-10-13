/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FF3E0.c (games/rac1/pal) through the version
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
/* functional: medium. Squared length of the vector at a0 (x*x + y*y + z*z) returned as a float, which the VU0 code computes in its accumulator; the summation order and rounding of the VU may differ from plain C. */
float FUN_L00_001ff430(float *v) {
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}
