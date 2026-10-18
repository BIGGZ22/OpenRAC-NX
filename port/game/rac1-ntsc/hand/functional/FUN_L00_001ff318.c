/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FF2C8.c (games/rac1) through the version
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
/* functional: medium. Checked against the US assembly 2026-10-17: reads s from $f12 and v from a1, scales xyz and keeps w. Written from the assembly, not compiled. Retail multiplies in VU0 single-precision arithmetic (its own rounding and denormal flush), which plain float multiplication may differ from in the last bit; the reads, writes and w-lane preservation are the same. */
void FUN_L00_001ff318(int unused, float s, float *v) {
    v[0] = v[0] * s;
    v[1] = v[1] * s;
    v[2] = v[2] * s;
}
