#include "include_asm.h"

extern unsigned char D_001A7A0A;

extern int D_001A79F0 __attribute__((sda));

extern int func_L00_002E3A90(int, int);

int func_L00_002E3B38(int x)
{
    int b = D_001A7A0A;
    int q;
    int v;

    if (b == 0) {
        q = func_L00_002E3A90(D_001A79F0, x) / 100;
        v = 3 - q;
        return b < v ? v : 1;
    }
    return 3;
}

INCLUDE_ASM("asm/levels", func_L00_002E3BA0);

INCLUDE_ASM("asm/levels", func_L00_002E3DB8);

INCLUDE_ASM("asm/levels", func_L00_002E54E0);
