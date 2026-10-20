#include "include_asm.h"

extern void *D_00133E74;

extern void *D_0013A308;

void *func_00115200(void)
{
    return D_00133E74;
}

void **func_00115210(void)
{
    return &D_0013A308;
}

INCLUDE_ASM("asm/core", func_00115220);

INCLUDE_ASM("asm/core", func_00115250);

ASM_FUNC("asm/core", func_00115340);

ASM_FUNC("asm/core", func_001153D4);

ASM_FUNC("asm/core", func_00115484);

ASM_FUNC("asm/core", func_00115544);

ASM_FUNC("asm/core", func_00115690);

ASM_FUNC("asm/core", func_001157AC);

INCLUDE_ASM("asm/core", func_001158F4);

ASM_FUNC("asm/core", func_00115AC0);

INCLUDE_ASM("asm/core", func_00115D38);

INCLUDE_ASM("asm/core", func_00115E28);

INCLUDE_ASM("asm/core", func_00115FC0);

INCLUDE_ASM("asm/core", func_00116300);

void func_001163A0(unsigned int value)
{
    *(unsigned int *)((unsigned char *)D_00133E74 + 0x58) = value;
}
