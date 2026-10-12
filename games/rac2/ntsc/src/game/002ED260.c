#include "include_asm.h"

#ifndef RAC2_T_U32_FE8672EA5
#define RAC2_T_U32_FE8672EA5
typedef unsigned int u32_Fe8672ea5;
#endif

extern u32_Fe8672ea5 D_L04_001B2AF0[];

extern u32_Fe8672ea5 D_L04_001AC18C __attribute__((sda));

extern float D_L04_001AC190 __attribute__((sda));

extern void func_00282C88(void *target);

void func_002ED260(u32_Fe8672ea5 object)
{
    u32_Fe8672ea5 cursor = object + 40952;
    int i;

    D_L04_001AC18C = object;
    for (i = 511; i >= 0; i--) {
        *(u32_Fe8672ea5 *)cursor = 0;
        cursor -= 80;
    }
    D_L04_001AC190 = 1.0f;
    func_00282C88(D_L04_001B2AF0);
}

INCLUDE_ASM("asm/game", func_002ED2B8);

INCLUDE_ASM("asm/game", func_002ED368);

INCLUDE_ASM("asm/game", func_002ED4B8);

INCLUDE_ASM("asm/game", func_002ED570);
