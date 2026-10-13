#include "include_asm.h"

extern int D_L00_001B22B0;
extern int D_L00_001BACEc;

int func_00288970(unsigned int x, int *out)
{
    if (x > 0x40000u) {
        *out = 0;
        return -1;
    }
    *out = D_L00_001BACEc + D_L00_001B22B0 - x;
    return 0;
}
