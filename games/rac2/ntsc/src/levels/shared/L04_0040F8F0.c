#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L04_0040F8F0);

INCLUDE_ASM("asm/levels", func_L04_00410518);

INCLUDE_ASM("asm/levels", func_L04_00410788);

/* shared native body of family 8c88be3bc489ab9e */
int func_L04_00410E48(int a0, int a1, float f12)
{
    int a2 = 0;

    if (*(int *)0x001A79F0 != 11) goto end;

    if (a0 == -1) {
        if (a1 >= 166) {
            if (a1 < 187) {
                if (f12 < 520.2f) {
                    a2 = 1;
                    goto end;
                }
            }
        }
    }

    if (a0 == 2) {
        if ((unsigned)(a1 - 1) < 6) {
            if (f12 < 537.0f) {
                a2 = 1;
                goto end;
            }
        }
        if ((unsigned)(a1 - 35) < 13) {
            if (f12 < 534.0f) {
                a2 = 1;
                goto end;
            }
        }
    }

end:
    return a2;
}

INCLUDE_ASM("asm/levels", func_L04_00410F10);

INCLUDE_ASM("asm/levels", func_L04_00410F80);

INCLUDE_ASM("asm/levels", func_L04_00411298);

INCLUDE_ASM("asm/levels", func_L04_00412130);

INCLUDE_ASM("asm/levels", func_L04_00412308);

INCLUDE_ASM("asm/levels", func_L04_00412C58);

INCLUDE_ASM("asm/levels", func_L04_00412E38);

INCLUDE_ASM("asm/levels", func_L04_00412EC0);

INCLUDE_ASM("asm/levels", func_L04_004135B8);

INCLUDE_ASM("asm/levels", func_L04_00413A38);

INCLUDE_ASM("asm/levels", func_L04_00414698);

INCLUDE_ASM("asm/levels", func_L04_00415E28);

INCLUDE_ASM("asm/levels", func_L04_00415EA8);

INCLUDE_ASM("asm/levels", func_L04_00415EE8);

INCLUDE_ASM("asm/levels", func_L04_00416018);

INCLUDE_ASM("asm/levels", func_L04_00416390);

INCLUDE_ASM("asm/levels", func_L04_004167B0);

INCLUDE_ASM("asm/levels", func_L04_00416E38);

INCLUDE_ASM("asm/levels", func_L04_00417028);
