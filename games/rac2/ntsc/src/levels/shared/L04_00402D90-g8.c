#include "include_asm.h"

/* Bodies whose retail bytes reach their globals through $gp: this file is built -G8. */

extern unsigned int D_L04_001A8ED0;

extern unsigned int D_L04_001B0908;

void func_L04_0040AC80(void)
{
    D_L04_001A8ED0 = 2;
    D_L04_001B0908 = 1;
}
