#include "include_asm.h"

extern int D_L01_001A8F40;
extern char D_L01_001A90C8 __attribute__((sda));
extern int D_L01_001A90D0 __attribute__((sda));
extern int func_00282948(char *);
extern void func_0029B810(int);

int func_L01_002F1888(void) {
    if (D_L01_001A8F40 == 0) {
        if (func_00282948(&D_L01_001A90C8))
            func_0029B810(0);
    }
    return D_L01_001A90D0;
}

extern int D_001B2088;
extern int D_001B2458;

int func_L01_00305E18(int a0)
{
    int a1 = a0;
    int p;

    if (D_001B2088 < 5 || (p = D_001B2458) == 0)
        return -1;
    return *(short *)(p + a1 * 0x28 + 0x24);
}

/* func_L01_002F1F70 is defined in L01_002A7248.c (this unit has no $gp access for it). */
