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

INCLUDE_ASM("asm/levels", func_L00_0032B500);

INCLUDE_ASM("asm/levels", func_L00_0032B9E8);

INCLUDE_ASM("asm/levels", func_L00_0032BBA8);

INCLUDE_ASM("asm/levels", func_L00_0032C5E8);
