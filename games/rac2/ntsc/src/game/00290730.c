#include "include_asm.h"

extern int ref_L00_002FC030_00 __attribute__((sda));

extern int ref_L00_002FC048_00 __attribute__((sda));

void func_00290730(void)
{
    if (ref_L00_002FC030_00 == 0)
        ref_L00_002FC030_00 = 1;
}

int func_00290748(void)
{
    return ref_L00_002FC048_00 == 3;
}

INCLUDE_ASM("asm/game", func_00290758);

INCLUDE_ASM("asm/game", func_002908D0);

INCLUDE_ASM("asm/game", func_00290978);
