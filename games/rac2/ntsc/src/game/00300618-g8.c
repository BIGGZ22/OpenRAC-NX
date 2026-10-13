#include "include_asm.h"

extern volatile unsigned int * volatile D_001B22A8;
extern void *F00300910_D_001B22A8 __attribute__((sda));
extern unsigned char D_001AD840 __attribute__((sda));

void func_00300910(void)
{
    D_001B22A8[0] = 0x30000003;
    D_001B22A8[1] = (unsigned int)&D_001AD840;
    D_001B22A8[2] = 0;
    D_001B22A8[3] = 0x50000003;
    F00300910_D_001B22A8 = (void *)(D_001B22A8 + 4);
}
