/* Carried from the PAL decompilation's nonmatching/game/func_001F98C0.c (games/rac1) through the version
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
extern float D_0015ED68 MACRO_ADDR;

/* FUN_001f96f8(x): returns (int)(0.5f + (float)x * D_0015ED68), truncated
   toward zero, the rounding of EE cvt.w.s. The 0.5 is the accumulator that
   adda.s loads from two copies of 0.25f before the madd. */
int FUN_001f96f8(int a) {
    float v = (float)a * D_0015ED68 + 0.5f;

    return (int)v;
}
