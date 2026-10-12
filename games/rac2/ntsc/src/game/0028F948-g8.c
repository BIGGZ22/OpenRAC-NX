#include "include_asm.h"

extern int D_L00_001A8F00;
extern int D_L00_001A9350 __attribute__((sda));
extern int D_L00_001A9090 __attribute__((sda));
extern int func_00282948(void *);
extern void func_0029B810(int);
int func_00290628(void)
{
    if (D_L00_001A8F00 == 0 && func_00282948(&D_L00_001A9350) != 0)
        func_0029B810(1);
    return D_L00_001A9090;
}
