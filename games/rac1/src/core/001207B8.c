#include "common.h"
#include "structs.h"

/*
 * core_text object 0x1207B8-0x1208E8. It follows libgcc's fp-bit.o, which
 * ends at 0x1207B8 once its dead-stripped functions are accounted for (see
 * tools/strip_dead.py); the end is retail's linker fill (0xCDCDCDCD).
 *
 * Both functions program hardware registers: func_001207B8 writes the
 * 0x10003Cxx block (VIF1) and func_00120858 the 0x10009000 block (DMA
 * channel 1), which suggests one of Sony's EE libraries. Not identified yet.
 */

INCLUDE_ASM("asm/nonmatchings/core_text", func_001207B8);

int func_00120858(void) {
    int busy;
    int stat;

    do {
        busy = 0;
        if (*(volatile int *)0x10009000 & 0x100) busy = 1;
        if (*(volatile int *)0x1000A000 & 0x100) busy |= 2;
        if (*(volatile int *)0x10003C00 & 3) busy |= 4;
        __asm__ __volatile__("cfc2.ni %0, $vi29\n\tnop" : "=r"(stat));
        if (stat & 0x100) busy |= 8;
        if (*(volatile int *)0x10003020 & 0xC00) busy |= 0x10;
    } while (busy);
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_001208E4);
