/* Carried from the PAL decompilation's nonmatching/functional/func_L02_002100E8.c (games/rac1/pal) through the version
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
/* functional: high. Written from the assembly, not compiled. Same stores in the same order (a1 to +0x80, a0 to +0x10, a2 to +0x20, 0x100 to +0x0 of the DMA block at 0x1000D400), then the read of 0x20100000 ored with 1. Retail leaves that value in $v1 rather than $v0; a caller that reads $v1 needs checking. */
int FUN_L02_002101b8(int a0, int a1, int a2) {
    *(volatile int *)0x1000D480 = a1;
    *(volatile int *)0x1000D410 = a0;
    *(volatile int *)0x1000D420 = a2;
    *(volatile int *)0x1000D400 = 0x100;
    return *(volatile int *)0x20100000 | 1;
}
