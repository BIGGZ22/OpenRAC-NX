/* Carried from the PAL decompilation's nonmatching/core/func_0012AAA8.c (games/rac1/pal) through the version
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
/* Bitstream peek: the top n bits of the 64-bit accumulator at +0x0, as an
   int. The shift count is taken mod 64 as the MIPS dsrlv does. */
int SignExtendPackedValue(void *arg0, int n) {
    unsigned long acc = *(unsigned long *)arg0;
    return (int)(acc >> ((64 - n) & 63));
}
