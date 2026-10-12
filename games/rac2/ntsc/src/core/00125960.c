#include "include_asm.h"

extern unsigned char D_00137E00;

void *func_00125960(void)
{
    return &D_00137E00;
}

INCLUDE_ASM("asm/core", func_0012596C);
