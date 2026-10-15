/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FF270.c (games/rac1) through the version
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
/* functional: medium. Written from the assembly, not compiled. Retail does this in VU0 single-precision arithmetic (its own rounding and denormal flush), which plain float subtraction may differ from in the last bit; the reads, writes and w-lane preservation are the same. */
void FUN_L00_001ff2c0(int unused, float *a, float *b) {
    a[0] = a[0] - b[0];
    a[1] = a[1] - b[1];
    a[2] = a[2] - b[2];
}
