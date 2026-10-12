#include "include_asm.h"

void func_0029B810(int index)
{
    extern char *D_001A8CFC __attribute__((sda));
    if ((unsigned int)index < 2) {
        char *base = D_001A8CFC + 0x38000;
        char *row = base + (index << 3);
        volatile int *p = (volatile int *)(row + 0x7390);
        volatile int *q = (volatile int *)(row + 0x7394);
        *p = 0;
        *q = 0;
    }
}