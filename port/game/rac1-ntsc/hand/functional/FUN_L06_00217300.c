/* Carried from the PAL decompilation's nonmatching/functional/func_L06_002172B0.c (games/rac1/pal) through the version
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
/* functional: high. Stores the architectural VU0 constant vf0 = (0, 0, 0, 1) as four floats at a0; vf0 is hardwired on the VU0, so the values are exact. Alignment of the 16-byte store is not modelled. */
void FUN_L06_00217300(float *p)
{
    p[0] = 0.0f;
    p[1] = 0.0f;
    p[2] = 0.0f;
    p[3] = 1.0f;
}
