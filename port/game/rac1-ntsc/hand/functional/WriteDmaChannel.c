/* Carried from the PAL decompilation's nonmatching/game/func_001F9AF0.c (games/rac1/pal) through the version
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
 * control registers) and then reads the status word at 0x20100000 (the
 * result is not returned). */
void WriteDmaChannel(unsigned int a0, unsigned int a1, unsigned int a2)
{
    unsigned int *ch = (unsigned int *)0x1000D000;
    unsigned int status;

    ch[0x80 / 4] = a1;
    ch[0x10 / 4] = a0;
    ch[0x00 / 4] = 0x100;
    ch[0x20 / 4] = a2;
    status = *(unsigned int *)0x20100000;
    status |= 1;
}
