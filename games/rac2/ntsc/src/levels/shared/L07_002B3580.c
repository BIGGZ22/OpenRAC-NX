#include "include_asm.h"

extern char D_L07_001FDEC0[];
extern void func_338D68();

INCLUDE_ASM("asm/levels", func_L07_002B3580);

INCLUDE_ASM("asm/levels", func_L07_002B5738);

INCLUDE_ASM("asm/levels", func_L07_00321D68);

INCLUDE_ASM("asm/levels", func_L07_00323038);

INCLUDE_ASM("asm/levels", func_L07_00323350);

int func_L07_00338D20(int a0)
{
    char *p;

    if (a0 == -1)
        return -1;
    p = *(char **)(D_L07_001FDEC0 + a0 * 4);
    if (p == 0)
        return -1;
    func_338D68(p);
    return 0;
}

INCLUDE_ASM("asm/levels", func_L07_0034AEC8);

INCLUDE_ASM("asm/levels", func_L07_003844C0);

INCLUDE_ASM("asm/levels", func_L07_003848D8);

INCLUDE_ASM("asm/levels", func_L07_003BA4D0);

INCLUDE_ASM("asm/levels", func_L07_003F6B88);

INCLUDE_ASM("asm/levels", func_L07_003F6F00);

INCLUDE_ASM("asm/levels", func_L07_003F90E8);

INCLUDE_ASM("asm/levels", func_L07_00409300);

INCLUDE_ASM("asm/levels", func_L07_00410B48);

INCLUDE_ASM("asm/levels", func_L07_00416510);
