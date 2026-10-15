#include "include_asm.h"

extern unsigned int ref_L00_002F9D30_00 __attribute__((sda));

void func_0028E440(void)
{
    if (ref_L00_002F9D30_00 != 0)
        --ref_L00_002F9D30_00;
}

INCLUDE_ASM("asm/game", func_0028E458);

extern int func_0028AB20(int);
extern unsigned short *D_001B188C;
extern short *D_001B1890;
extern unsigned int *D_001B1898;
extern unsigned int *D_001B1894;

int func_0028E588(int key, int index)
{
    int number = func_0028AB20(key);
    unsigned short *row = (unsigned short *)(number * 8 + (unsigned int)D_001B188C);
    if (row[0] == 65535)
        goto fail;
    if (index >= row[1])
        goto fail;
    {
        unsigned int mask = 0x80000000u;
        int offset = row[2] + index;
        short *pair = (short *)(offset * 4 + (unsigned int)*(short *volatile *)&D_001B1890);
        if (D_001B1898[pair[0] * 2] & mask)
            goto fail;
        return (D_001B1894[pair[1] * 2] & mask) ? 0 : offset;
    }
fail:
    return 0;
}

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
