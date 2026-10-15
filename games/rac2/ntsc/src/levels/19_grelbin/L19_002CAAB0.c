#include "include_asm.h"

extern int D_L19_001A8F00 __attribute__((sda));

INCLUDE_ASM("asm/levels", func_L19_002CAAB0);

INCLUDE_ASM("asm/levels", func_L19_002FA1A8);

INCLUDE_ASM("asm/levels", func_L19_00380918);

void func_L19_00380A98(char *a0)
{
    if (D_L19_001A8F00 == 2) {
        *(int *)(a0 + 0x98) = 0;
        *(unsigned short *)(a0 + 0x34) = (*(unsigned short *)(a0 + 0x34) | 0x41) & 0xEFFF;
    } else {
        *(unsigned short *)(a0 + 0x34) = (*(unsigned short *)(a0 + 0x34) & 0xFFBE) | 0x1000;
        *(int *)(a0 + 0x98) = *(int *)(*(char **)(a0 + 0x24) + 0x10);
    }
}

INCLUDE_ASM("asm/levels", func_L19_0038B580);

INCLUDE_ASM("asm/levels", func_L19_0038B998);

INCLUDE_ASM("asm/levels", func_L19_00390F78);

INCLUDE_ASM("asm/levels", func_L19_003C7618);

INCLUDE_ASM("asm/levels", func_L19_003D2E38);
