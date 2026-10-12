#include "include_asm.h"

extern void func_00282D30_2(char *dst, char *src, float scale);

INCLUDE_ASM("asm/levels", func_L11_004465F8);

INCLUDE_ASM("asm/levels", func_L11_00446C78);

INCLUDE_ASM("asm/levels", func_L11_00446EB8);

INCLUDE_ASM("asm/levels", func_L11_00447018);

INCLUDE_ASM("asm/levels", func_L11_004471A8);

INCLUDE_ASM("asm/levels", func_L11_00447360);

INCLUDE_ASM("asm/levels", func_L11_004475A0);

INCLUDE_ASM("asm/levels", func_L11_004484A8);

INCLUDE_ASM("asm/levels", func_L11_00448848);

INCLUDE_ASM("asm/levels", func_L11_00448908);

INCLUDE_ASM("asm/levels", func_L11_00448AA8);

INCLUDE_ASM("asm/levels", func_L11_00448CE0);

INCLUDE_ASM("asm/levels", func_L11_00448D28);

INCLUDE_ASM("asm/levels", func_L11_00448DF8);

INCLUDE_ASM("asm/levels", func_L11_00449098);

INCLUDE_ASM("asm/levels", func_L11_004492A0);

INCLUDE_ASM("asm/levels", func_L11_00449330);

INCLUDE_ASM("asm/levels", func_L11_004496A0);

INCLUDE_ASM("asm/levels", func_L11_00449878);

INCLUDE_ASM("asm/levels", func_L11_00449970);

INCLUDE_ASM("asm/levels", func_L11_00449AD8);

INCLUDE_ASM("asm/levels", func_L11_00449C70);

INCLUDE_ASM("asm/levels", func_L11_0044AFC8);

INCLUDE_ASM("asm/levels", func_L11_0044BD68);

INCLUDE_ASM("asm/levels", func_L11_0044C0F0);

INCLUDE_ASM("asm/levels", func_L11_0044C180);

INCLUDE_ASM("asm/levels", func_L11_0044C220);

INCLUDE_ASM("asm/levels", func_L11_0044C268);

INCLUDE_ASM("asm/levels", func_L11_0044C328);

INCLUDE_ASM("asm/levels", func_L11_0044C4F0);

INCLUDE_ASM("asm/levels", func_L11_0044C688);

INCLUDE_ASM("asm/levels", func_L11_0044C818);

INCLUDE_ASM("asm/levels", func_L11_0044C850);

INCLUDE_ASM("asm/levels", func_L11_00450328);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00450748(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L11_00451178);

INCLUDE_ASM("asm/levels", func_L11_00452450);

INCLUDE_ASM("asm/levels", func_L11_00452810);

INCLUDE_ASM("asm/levels", func_L11_004528A0);

INCLUDE_ASM("asm/levels", func_L11_00452C58);

INCLUDE_ASM("asm/levels", func_L11_00452DF8);

INCLUDE_ASM("asm/levels", func_L11_00452F30);

INCLUDE_ASM("asm/levels", func_L11_004530C0);

INCLUDE_ASM("asm/levels", func_L11_004530E0);
