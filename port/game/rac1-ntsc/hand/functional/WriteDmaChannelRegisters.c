/* Carried from the PAL decompilation's nonmatching/game/func_0020C210.c (games/rac1/pal) through the version
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
/* WriteDmaChannelRegisters(madr, qwc, tadr): sets channel 8's MADR, QWC and TADR, then
   CHCR = 0x100 (STR) to start the transfer; returns 0x100. Stores in the
   retail order: TADR, QWC, MADR, CHCR. */
int WriteDmaChannelRegisters(unsigned int madr, unsigned int qwc, unsigned int tadr)
{
    *(unsigned int *)0x1000D480 = tadr;
    *(unsigned int *)0x1000D420 = qwc;
    *(unsigned int *)0x1000D410 = madr;
    *(unsigned int *)0x1000D400 = 0x100;
    return 0x100;
}
