#include "include_asm.h"

extern unsigned int ref_L00_002F9D30_00 __attribute__((sda));

void func_0028E440(void)
{
    if (ref_L00_002F9D30_00 != 0)
        --ref_L00_002F9D30_00;
}

INCLUDE_ASM("asm/game", func_0028E458);

INCLUDE_ASM("asm/game", func_0028E588);

INCLUDE_ASM("asm/game", func_0028E640);

INCLUDE_ASM("asm/game", func_0028E868);

INCLUDE_ASM("asm/game", func_0028EA58);

INCLUDE_ASM("asm/game", func_0028ECD8);

INCLUDE_ASM("asm/game", func_0028EE80);

INCLUDE_ASM("asm/game", func_0028EE98);

INCLUDE_ASM("asm/game", func_0028F078);

INCLUDE_ASM("asm/game", func_0028F278);

INCLUDE_ASM("asm/game", func_0028F408);

INCLUDE_ASM("asm/game", func_0028F410);

INCLUDE_ASM("asm/game", func_0028F788);
