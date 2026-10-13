#include "include_asm.h"

extern void func_002F0538(int);

extern void func_002E7D48(int, int, int, int);

extern unsigned int D_L00_001B18D4;

extern short D_L00_001B267E;

extern void func_002E5C40(void);

void func_L00_0037F790(void)
{
    func_002F0538(0);
    func_002E7D48(0, 0, 0, 0x80);
    if (D_L00_001B267E != 0)
        D_L00_001B18D4 = 0x100FF;
    else
        D_L00_001B18D4 = 0x70;
    func_002E5C40();
}
