#include "include_asm.h"

/* Read the word at offset 0x 18 through the pointer stored at owner offset 0x 68. */
unsigned int func_L00_0032C638(unsigned int param_1)
{
    unsigned int inner_ptr;
    unsigned int result;

    inner_ptr = *(unsigned int *)(param_1 + 0x68);
    result = *(unsigned int *)(inner_ptr + 0x18);
    return result;
}

INCLUDE_ASM("asm/levels", func_L00_0032C9B8);

INCLUDE_ASM("asm/levels", func_L00_0032CA28);

INCLUDE_ASM("asm/levels", func_L00_0032CB30);

INCLUDE_ASM("asm/levels", func_L00_0032CF60);

void func_L00_0032D100(int value, long address) {
    int *target = (int *)(int)address;
    if (target != 0)
        *target = value;
}

INCLUDE_ASM("asm/levels", func_L00_0032D208);

INCLUDE_ASM("asm/levels", func_L00_0032D310);

INCLUDE_ASM("asm/levels", func_L00_0032EF10);
