#include "include_asm.h"
#include "common.h"

typedef struct {
    u8 pad[0x1FB0];
    f32 f1FB0;
    f32 f1FB4;
} D00189E20_View;

extern D00189E20_View D_00189E20;

extern void func_00282948();

INCLUDE_ASM("asm/levels", func_L01_003835D8);

INCLUDE_ASM("asm/levels", func_L01_003839F0);

INCLUDE_ASM("asm/levels", func_L01_00388FD0);

INCLUDE_ASM("asm/levels", func_L01_003A5BB8);

INCLUDE_ASM("asm/levels", func_L01_003A6028);

INCLUDE_ASM("asm/levels", func_L01_003A6280);

INCLUDE_ASM("asm/levels", func_L01_003A6350);

void func_L01_003A6448(float *arg0, float *arg1) {
    *arg0 = -D_00189E20.f1FB0;
    *arg1 = -D_00189E20.f1FB4;
    if (*arg0 > 1.0f)
        *arg0 = 1.0f;
    else if (*arg0 < -1.0f)
        *arg0 = -1.0f;
    if (*arg1 > 1.0f)
        *arg1 = 1.0f;
    else if (*arg1 < -1.0f)
        *arg1 = -1.0f;
}

INCLUDE_ASM("asm/levels", func_L01_003A64F0);

INCLUDE_ASM("asm/levels", func_L01_003A6638);

INCLUDE_ASM("asm/levels", func_L01_003B1F08);

INCLUDE_ASM("asm/levels", func_L01_003B4200);

INCLUDE_ASM("asm/levels", func_L01_003B4480);

INCLUDE_ASM("asm/levels", func_L01_003B45D8);

void func_L01_003B46C8(char **arg0, int *arg1) {
    int i;

    if (*arg1 <= 0)
        return;
    i = 0;
    do {
        *(int *)(arg0[i] + 0x9C) = 0;
        i++;
    } while (i < *arg1);
}

INCLUDE_ASM("asm/levels", func_L01_003B4700);

INCLUDE_ASM("asm/levels", func_L01_003B49D8);

INCLUDE_ASM("asm/levels", func_L01_003B4AD0);

void func_L01_003B4BD8(u8 *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);

    if (arg0[0x21] != 0xFF) {
        if (*(int *)(s0 + 0x340) != 0) {
            func_00282948(*(int **)(s0 + 0x33C));
        }
    }
    *(int *)(s0 + 0x340) = 0;
}

int func_L01_003B4C20(u8 *arg0) {
    char *a1 = *(char **)(arg0 + 0x68);
    int r = 1;

    if (arg0[0x21] != 0xFF) {
        if (*(int *)(a1 + 0x340) == 0) {
            int *p = *(int **)(a1 + 0x33C);
            *p = *p + 1;
        }
    }
    *(int *)(a1 + 0x340) = r;
    return r;
}

INCLUDE_ASM("asm/levels", func_L01_003B4C58);

INCLUDE_ASM("asm/levels", func_L01_003B50C8);

INCLUDE_ASM("asm/levels", func_L01_003B5310);

INCLUDE_ASM("asm/levels", func_L01_003B53C8);

INCLUDE_ASM("asm/levels", func_L01_003B57A8);

INCLUDE_ASM("asm/levels", func_L01_003B5878);

INCLUDE_ASM("asm/levels", func_L01_003B9F30);

/* func_L01_003BAFB0 lives in the -g8 sibling file (it reads a float through $gp). */

INCLUDE_ASM("asm/levels", func_L01_003BAFB8);
