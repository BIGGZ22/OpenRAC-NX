#include "include_asm.h"

typedef unsigned short u16;

typedef int s32;

extern s32 D_L04_001B1CB8 __attribute__((sda));

extern s32 D_L04_001B1CBC __attribute__((sda));

extern s32 D_L04_001B1CCC __attribute__((sda));

extern void func_L04_0046CD68(char *a0, s32 a1);

/* Clear the mode word, the sub-state counter and the three timers of a queue entry. */
void func_L04_0046CE28(char *a0)
{
    char *s0 = a0;

    func_L04_0046CD68(s0, 0);
    *(u16 *)(s0 + 0x7E) = 0;
    D_L04_001B1CB8 = -1;
    D_L04_001B1CBC = -1;
    D_L04_001B1CCC = -1;
}
