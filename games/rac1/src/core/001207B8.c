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

typedef unsigned int u128_t __attribute__((mode(TI)));
extern u128_t D_001313A0;
extern u128_t D_001313B0;

void func_001207B8(void) {
    *(volatile int *)0x10003C10 = 1;
    *(volatile int *)0x10003C20 = 2;
    __asm__ __volatile__(
        "move $4, $0\n\tsync\n\t"
        "cfc2.ni $4, $vi28\n\tnop\n\tori $4, $4, 0x200\n\tctc2.ni $4, $vi28\n\tsync.p\n"
        "1:\n\tcfc2.ni $4, $vi29\n\tnop\n\tnop\n\tnop\n\tandi $4, $4, 0x100\n\tbnez $4, 1b\n\t"
        "addiu $4, $0, 0x404\n\tctc2.ni $4, $vi28\n\tnop"
        : : : "$4");
    *(volatile u128_t *)0x10005000 = D_001313A0;
    *(volatile u128_t *)0x10005000 = D_001313B0;
    *(volatile int *)0x10003000 = 1;
}

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
