#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L09_002B7238);

INCLUDE_ASM("asm/levels", func_L09_00325C58);

INCLUDE_ASM("asm/levels", func_L09_00326E90);

INCLUDE_ASM("asm/levels", func_L09_00329028);

INCLUDE_ASM("asm/levels", func_L09_00329178);

INCLUDE_ASM("asm/levels", func_L09_003364E0);

INCLUDE_ASM("asm/levels", func_L09_0033E658);

INCLUDE_ASM("asm/levels", func_L09_0033E690);

INCLUDE_ASM("asm/levels", func_L09_0033E7A0);

INCLUDE_ASM("asm/levels", func_L09_0033E8B0);

INCLUDE_ASM("asm/levels", func_L09_0037FCE8);

INCLUDE_ASM("asm/levels", func_L09_003806A0);

INCLUDE_ASM("asm/levels", func_L09_003A01D8);

INCLUDE_ASM("asm/levels", func_L09_003A06B0);

INCLUDE_ASM("asm/levels", func_L09_003AE8C8);

INCLUDE_ASM("asm/levels", func_L09_003AEA88);

INCLUDE_ASM("asm/levels", func_L09_003AEE10);

INCLUDE_ASM("asm/levels", func_L09_003AEE80);

INCLUDE_ASM("asm/levels", func_L09_003AF060);

INCLUDE_ASM("asm/levels", func_L09_003AF508);

INCLUDE_ASM("asm/levels", func_L09_003AF6B0);

INCLUDE_ASM("asm/levels", func_L09_003AF9E8);

INCLUDE_ASM("asm/levels", func_L09_003B6550);

INCLUDE_ASM("asm/levels", func_L09_003E99E0);

INCLUDE_ASM("asm/levels", func_L09_003E9AA0);

INCLUDE_ASM("asm/levels", func_L09_003E9E60);

INCLUDE_ASM("asm/levels", func_L09_003EA058);

INCLUDE_ASM("asm/levels", func_L09_003EA108);

INCLUDE_ASM("asm/levels", func_L09_003EA2D0);

INCLUDE_ASM("asm/levels", func_L09_003EA360);

INCLUDE_ASM("asm/levels", func_L09_003EA798);

INCLUDE_ASM("asm/levels", func_L09_003EA9D0);

INCLUDE_ASM("asm/levels", func_L09_003EAA18);

INCLUDE_ASM("asm/levels", func_L09_003EAB20);

INCLUDE_ASM("asm/levels", func_L09_003EAF60);

INCLUDE_ASM("asm/levels", func_L09_003F65E0);

INCLUDE_ASM("asm/levels", func_L09_003F7158);

INCLUDE_ASM("asm/levels", func_L09_003F71C8);

INCLUDE_ASM("asm/levels", func_L09_003F72A8);

INCLUDE_ASM("asm/levels", func_L09_003F7370);

INCLUDE_ASM("asm/levels", func_L09_003F73E8);

INCLUDE_ASM("asm/levels", func_L09_003F7430);

INCLUDE_ASM("asm/levels", func_L09_003F7468);

/* shared native body of family cd6f3d9eca5f8edd */
/* attempt 1 - shape read straight off the disassembly:
   save object's word 152 from its owner's field 16, then repeat for a second
   object found through two pointer hops, skipping the sentinel byte values. */

void func_L09_003F74A8(char *p)
{
    char *q;
    char *r;
    char *s;
    int v;

    if (p == 0)
        return;
    if (*(unsigned char *)(p + 32) == 254)
        return;
    if (*(unsigned char *)(p + 32) == 253)
        return;

    q = *(char **)(p + 36);
    r = *(char **)(p + 104);
    *(int *)(p + 152) = *(int *)(q + 16);

    s = *(char **)(r + 760);
    if (s == 0)
        return;
    if (*(short *)(s + 170) != 3342)
        return;
    if (*(unsigned char *)(s + 32) == 254)
        return;
    if (*(unsigned char *)(s + 32) == 253)
        return;

    v = *(int *)(s + 36);
    *(int *)(s + 152) = *(int *)(v + 16);
}

INCLUDE_ASM("asm/levels", func_L09_003F7518);

INCLUDE_ASM("asm/levels", func_L09_003F7788);

INCLUDE_ASM("asm/levels", func_L09_003F77D0);

INCLUDE_ASM("asm/levels", func_L09_003F7840);

INCLUDE_ASM("asm/levels", func_L09_003F7A88);

INCLUDE_ASM("asm/levels", func_L09_003F7B28);

INCLUDE_ASM("asm/levels", func_L09_003F7C50);

INCLUDE_ASM("asm/levels", func_L09_003F7CE0);

INCLUDE_ASM("asm/levels", func_L09_003F8598);

INCLUDE_ASM("asm/levels", func_L09_003F87D8);

INCLUDE_ASM("asm/levels", func_L09_003F8858);

INCLUDE_ASM("asm/levels", func_L09_003F8898);

INCLUDE_ASM("asm/levels", func_L09_003F8960);

INCLUDE_ASM("asm/levels", func_L09_003F8AE0);

INCLUDE_ASM("asm/levels", func_L09_003F9040);

INCLUDE_ASM("asm/levels", func_L09_003F9108);
