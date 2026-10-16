/* Carried from port/game/rac1-pal/hand/level_liquid_grid.c through the version
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
/*
 * FUN_L02_002101b8(from, to, quadwords): a transfer into the scratchpad on the toSPR DMA channel
 * (0x1000D400: SADR, MADR, QWC, then CHCR started), retail's 0x30 bytes of assembly. The port has
 * no DMA: the copy is done at once (the channel is idle by the time anything reads the scratchpad).
 * The liquid grid sends its strip order to 0x70002800 with it.
 */
void FUN_L02_002101b8(unsigned char *from, unsigned int to, int quadwords) {
    unsigned char *out = (unsigned char *)(0x70000000u | (to & 0x3FF0u));
    int i;
    for (i = 0; i < quadwords * 16; i++) {
        out[i] = from[i];
    }
}
