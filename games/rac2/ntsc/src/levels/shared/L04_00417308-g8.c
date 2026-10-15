#include "include_asm.h"

typedef unsigned short u16;

typedef int s32;

extern s32 D_L04_001B1CB8 __attribute__((sda));

extern s32 D_L04_001B1CBC __attribute__((sda));

extern s32 D_L04_001B1CCC __attribute__((sda));

extern float D_L04_001B1C8C __attribute__((sda));

extern float D_L04_001B1C90 __attribute__((sda));

extern float D_L04_001B1C94 __attribute__((sda));

extern float D_L04_001B1C98 __attribute__((sda));

extern s32 D_L04_001B1CC4 __attribute__((sda));

extern float func_30EEF8(char *);

extern void func_L04_0046CE68(char *);

INCLUDE_ASM("asm/levels", func_L04_0046CD68);

/* Arm one queue entry: seed its timers, scale them when it repeats, and start it. */
void func_L04_0046CE28(char *a0)
{
    char *s0 = a0;

    func_L04_0046CD68(s0, 0);
    *(u16 *)(s0 + 0x7E) = 0;
    D_L04_001B1CB8 = -1;
    D_L04_001B1CBC = -1;
    D_L04_001B1CCC = -1;
}

