#include "include_asm.h"

extern int D_001AA100 __attribute__((sda));
extern int D_001B1D50[];

int func_002B4BE8(void) {
    int count = D_001AA100;
    if (count != 0)
        return D_001B1D50[count - 1];
    return -2;
}
