#include "include_asm.h"

extern int ref_L01_00320AE8_00 __attribute__((sda));

/* Store the observed raw 32-bit input in the GP-relative cell.
   The original owner and semantic value remain unknown. */
extern unsigned int D_L00_001AA430;

extern int ref_L04_0034E130_00 __attribute__((sda));

extern int ref_L01_00320B20_00;

int func_L00_0032B270(void)
{
    return ref_L01_00320AE8_00 > 0;
}

void func_L00_0032B280(unsigned int value)
{
    D_L00_001AA430 = value;
}

void func_L00_0032B288(void)
{
    int value = (int)((unsigned int)ref_L04_0034E130_00 - 1u);
    ref_L04_0034E130_00 = value;
    if (value < 0)
        ref_L04_0034E130_00 = 0;
}

void func_L00_0032B2A8(int value)
{
    ref_L01_00320B20_00 = value;
}

extern int D_L00_001A8F00;
extern int D_L00_001A8F04;
extern int D_L00_001A8F08;
extern int D_L00_001AA440 __attribute__((sda));
extern int D_L00_001AA444 __attribute__((sda));

/* records the raw input in the absolute state block, parks a -2 marker beside it
   and clears the two small-data cells that follow.  The two identical stores are
   written in the order this cc1 emits them (it swaps them otherwise). */
void func_L00_0032B500(int value)
{
    D_L00_001A8F00 = value;
    D_L00_001A8F08 = -2;
    D_L00_001A8F04 = value;
    D_L00_001AA440 = 0;
    D_L00_001AA444 = 0;
}

INCLUDE_ASM("asm/levels", func_L00_0032B9E8);

INCLUDE_ASM("asm/levels", func_L00_0032BBA8);

/* The record at 0x88 of the second argument decides: arm it with the four
   sub-addresses and light bit 0x40 on the record's 0x94 word when it takes. */
extern int ref_L00_00320DE0_00();

void func_L00_0032C5E8(char *a0, char *a1)
{
    char *s0 = a1;

    if (*(int *)(s0 + 0x88) != 0) {
        if (ref_L00_00320DE0_00(a0, *(int *)(s0 + 0x88), a0 + 0x10, a0 + 0xF0,
                                a0 + 0x10, a0 + 0xF0) != 0)
            *(int *)(s0 + 0x94) = *(int *)(s0 + 0x94) | 0x40;
    }
}
