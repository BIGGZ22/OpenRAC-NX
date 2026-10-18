/* functional: high. Checked against the US assembly (2026-10-17): the four channel stores and the
 * CHCR start value match; the return value is retail's $v0 (0x100, set before the status read), now
 * returned as int so the caller's int declaration matches; the status read is volatile so it stays.
 * Carried from the PAL decompilation's nonmatching/game/func_001F9AF0.c (games/rac1) through the version
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
/* Sets up a DMA channel at 0x1000D000 (the channel's address, count and
 * control registers) and then reads the status word at 0x20100000. Retail
 * leaves 0x100 in $v0 (the CHCR start value) and the status in $v1; the
 * callers that declare an int result see the 0x100. */
int WriteDmaChannel(unsigned int a0, unsigned int a1, unsigned int a2)
{
    volatile unsigned int *ch = (volatile unsigned int *)0x1000D000;

    ch[0x80 / 4] = a1;
    ch[0x10 / 4] = a0;
    ch[0x00 / 4] = 0x100;
    ch[0x20 / 4] = a2;
    (void)*(volatile unsigned int *)0x20100000;
    return 0x100;
}
