#include "include_asm.h"

typedef unsigned char u8;
typedef float f32;

/* Family cbc1388ebe801e68 (116 B, 2 placements): arm one animation slot on a
 * resident object.  Two global guards, a "current slot" guard, then five
 * resident stores in the retail order.
 *
 * The indexed read `p + a1 * 4 + 1152` must be written as a named byte pointer
 * (`w = p + a1 * 4`) followed by the load at `w + 1152`: when the 1152 is folded
 * into the same PLUS chain, gcc canonicalises the commutative `addu` with the
 * scaled index as the first source operand, one byte away from retail.
 */

extern u8 D_001A63A8_4[];

INCLUDE_ASM("asm/levels", func_L22_003A5018);

INCLUDE_ASM("asm/levels", func_L22_003A5BD8);

INCLUDE_ASM("asm/levels", func_L22_003A5F90);

INCLUDE_ASM("asm/levels", func_L22_003A6128);

INCLUDE_ASM("asm/levels", func_L22_003A6230);

/* shared native body of family cbc1388ebe801e68 */
void func_L22_003A6470(u8 *a0, int a1) {
    u8 *g = D_001A63A8_4;
    u8 *p = *(u8 **)(a0 + 104);
    int bit;
    if (*(int *)(g + 104) != 0) return;
    if (*(int *)(g + 36) != -1) return;
    if (a1 == *(int *)(p + 1256)) return;
    bit = 1 << a1;
    { u8 *w = p + a1 * 4; *(int *)(g + 36) = (*(int *)(w + 1152) & 0x3fff) + 2000; }
    *(int *)(g + 40) = 0;
    *(int *)(p + 1256) = a1;
    *(int *)(p + 1252) = 1800;
    *(int *)(p + 1248) |= bit;
}

INCLUDE_ASM("asm/levels", func_L22_003A64E8);

INCLUDE_ASM("asm/levels", func_L22_003A65E0);

INCLUDE_ASM("asm/levels", func_L22_003A6640);

extern u8 D_00189E20[];
extern f32 func_00282E48(void *a0, void *a1);
extern void func_L22_002B70F8(int a0, int a1, void *a2);

void func_L22_003A8BA0(u8 *a0) {
    if (D_00189E20[0x22B4] != 0) return;
    if (func_00282E48(D_00189E20 + 0x80, a0 + 0x10) < 4.0f) {
        func_L22_002B70F8(2, 0x56, a0);
    }
}

INCLUDE_ASM("asm/levels", func_L22_003AD090);

INCLUDE_ASM("asm/levels", func_L22_003B8FE8);

INCLUDE_ASM("asm/levels", func_L22_003B9128);

INCLUDE_ASM("asm/levels", func_L22_003B9358);

INCLUDE_ASM("asm/levels", func_L22_003C9880);

INCLUDE_ASM("asm/levels", func_L22_003C9B60);

INCLUDE_ASM("asm/levels", func_L22_003C9D48);

INCLUDE_ASM("asm/levels", func_L22_003D4320);

extern void func_L22_003D4320(u8 *a0, int a1, int a2);

void func_L22_003D4428(u8 *a0) {
    int v = *(int *)(a0 + 0x68);
    func_L22_003D4320(a0, v, v);
    *(u8 *)(a0 + 0x30) = 0xFF;
}

INCLUDE_ASM("asm/levels", func_L22_003F45D8);

INCLUDE_ASM("asm/levels", func_L22_003F52D8);

INCLUDE_ASM("asm/levels", func_L22_003F54B8);

INCLUDE_ASM("asm/levels", func_L22_003F5580);

INCLUDE_ASM("asm/levels", func_L22_003F5688);

INCLUDE_ASM("asm/levels", func_L22_003F58F8);

INCLUDE_ASM("asm/levels", func_L22_003F85A0);

INCLUDE_ASM("asm/levels", func_L22_003F8658);

INCLUDE_ASM("asm/levels", func_L22_003FB1F8);

INCLUDE_ASM("asm/levels", func_L22_003FB310);

INCLUDE_ASM("asm/levels", func_L22_003FB9B8);

INCLUDE_ASM("asm/levels", func_L22_003FBBF0);

INCLUDE_ASM("asm/levels", func_L22_003FBF00);

INCLUDE_ASM("asm/levels", func_L22_003FBFD0);

INCLUDE_ASM("asm/levels", func_L22_003FC048);

INCLUDE_ASM("asm/levels", func_L22_003FC338);

INCLUDE_ASM("asm/levels", func_L22_003FC778);

INCLUDE_ASM("asm/levels", func_L22_003FC780);

INCLUDE_ASM("asm/levels", func_L22_003FC870);

INCLUDE_ASM("asm/levels", func_L22_003FCF08);

INCLUDE_ASM("asm/levels", func_L22_003FD1E8);

INCLUDE_ASM("asm/levels", func_L22_003FD2D8);

INCLUDE_ASM("asm/levels", func_L22_003FD958);

INCLUDE_ASM("asm/levels", func_L22_003FDB90);

INCLUDE_ASM("asm/levels", func_L22_0040DD40);

INCLUDE_ASM("asm/levels", func_L22_0040E2B0);
