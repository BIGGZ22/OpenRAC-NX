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

INCLUDE_ASM("asm/core", func_00115228);

INCLUDE_ASM("asm/core", func_00115250);

INCLUDE_ASM("asm/core", func_00115340);

INCLUDE_ASM("asm/core", func_001153D4);

INCLUDE_ASM("asm/core", func_00115484);

INCLUDE_ASM("asm/core", func_00115544);

INCLUDE_ASM("asm/core", func_00115690);

INCLUDE_ASM("asm/core", func_001157AC);

INCLUDE_ASM("asm/core", func_001158F4);

INCLUDE_ASM("asm/core", func_00115AC0);

INCLUDE_ASM("asm/core", func_00115C90);

INCLUDE_ASM("asm/core", func_00115CF0);

INCLUDE_ASM("asm/core", func_00115D38);

INCLUDE_ASM("asm/core", func_00115D48);

INCLUDE_ASM("asm/core", func_00115DA8);

INCLUDE_ASM("asm/core", func_00115E28);

INCLUDE_ASM("asm/core", func_00115E68);

INCLUDE_ASM("asm/core", func_00115E90);

INCLUDE_ASM("asm/core", func_00115F28);

INCLUDE_ASM("asm/core", func_00115F78);

INCLUDE_ASM("asm/core", func_00115FC0);

INCLUDE_ASM("asm/core", func_00116300);

INCLUDE_ASM("asm/core", func_00116360);

void func_001163A0(unsigned int value)
{
    *(unsigned int *)((unsigned char *)D_00133E74 + 0x58) = value;
}
