#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L00_003DC2D0);

extern int D_L00_001B043C __attribute__((sda));

extern int D_L00_001B2B44 __attribute__((sda));

int func_L00_003DE260(void)
{
    char *p = (char *)D_L00_001B043C;

    if (p == 0)
        return 0;
    return (unsigned int)((unsigned char)p[0x20] - 5) < 3;
}

int func_L00_003DF108(void)
{
    if (D_L00_001B043C == 0)
        return 0;
    return D_L00_001B2B44;
}

INCLUDE_ASM("asm/levels", func_L00_003DF6B0);
