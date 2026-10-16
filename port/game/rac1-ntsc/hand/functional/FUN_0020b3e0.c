/* Carried from the PAL decompilation's nonmatching/game/func_0020C230.c (games/rac1) through the version
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
/* DmaToSprSync(void): waits while channel 8's CHCR has STR (0x100) set. */
void FUN_0020b3e0(void)
{
    /* fix: volatile, so the register is re-read every iteration as retail does (0020C238 lw in the loop) */
    while (*(volatile unsigned int *)0x1000D400 & 0x100) {
    }
}
