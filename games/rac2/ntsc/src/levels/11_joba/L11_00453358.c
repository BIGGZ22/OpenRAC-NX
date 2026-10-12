#include "include_asm.h"

extern void func_00282D30_2(char *dst, char *src, float scale);

INCLUDE_ASM("asm/levels", func_L11_00453358);

INCLUDE_ASM("asm/levels", func_L11_00454508);

INCLUDE_ASM("asm/levels", func_L11_00454AE0);

INCLUDE_ASM("asm/levels", func_L11_00454B60);

INCLUDE_ASM("asm/levels", func_L11_00454C78);

INCLUDE_ASM("asm/levels", func_L11_00454DD8);

INCLUDE_ASM("asm/levels", func_L11_00454F18);

INCLUDE_ASM("asm/levels", func_L11_00455DF8);

INCLUDE_ASM("asm/levels", func_L11_00456010);

INCLUDE_ASM("asm/levels", func_L11_00456070);

INCLUDE_ASM("asm/levels", func_L11_00456378);

INCLUDE_ASM("asm/levels", func_L11_00456438);

INCLUDE_ASM("asm/levels", func_L11_00456540);

INCLUDE_ASM("asm/levels", func_L11_004565D8);

INCLUDE_ASM("asm/levels", func_L11_004566C0);

INCLUDE_ASM("asm/levels", func_L11_00456938);

INCLUDE_ASM("asm/levels", func_L11_00456A48);

INCLUDE_ASM("asm/levels", func_L11_00456C20);

INCLUDE_ASM("asm/levels", func_L11_00456EA0);

INCLUDE_ASM("asm/levels", func_L11_00457600);

INCLUDE_ASM("asm/levels", func_L11_004577F8);

INCLUDE_ASM("asm/levels", func_L11_004580E8);

INCLUDE_ASM("asm/levels", func_L11_00458A98);

INCLUDE_ASM("asm/levels", func_L11_00458C38);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00458D88(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L11_00458F58);

INCLUDE_ASM("asm/levels", func_L11_00458F90);

INCLUDE_ASM("asm/levels", func_L11_004590F0);

INCLUDE_ASM("asm/levels", func_L11_00459640);

INCLUDE_ASM("asm/levels", func_L11_00459D48);

INCLUDE_ASM("asm/levels", func_L11_00459E48);

/* shared native body of family a04928c6 */
void func_L11_00459EA0(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L11_00459F30);

INCLUDE_ASM("asm/levels", func_L11_0045A3C8);

INCLUDE_ASM("asm/levels", func_L11_0045A578);

INCLUDE_ASM("asm/levels", func_L11_0045A8C0);

INCLUDE_ASM("asm/levels", func_L11_0045B5C8);

INCLUDE_ASM("asm/levels", func_L11_0045B6C8);

INCLUDE_ASM("asm/levels", func_L11_0045FA68);

INCLUDE_ASM("asm/levels", func_L11_0045FDB0);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00460038(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L11_004600A0);

INCLUDE_ASM("asm/levels", func_L11_00461308);
