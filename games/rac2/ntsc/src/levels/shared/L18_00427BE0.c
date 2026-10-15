#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned short u16;

typedef signed short s16;

typedef signed int s32;

INCLUDE_ASM("asm/levels", func_L18_00427BE0);

INCLUDE_ASM("asm/levels", func_L18_00428048);

/* Reports zero after handing the level's measured list to the shared test. */
extern void func_L00_003B9310(void *);

extern u8 D_L18_002571A0[];
INCLUDE_ASM("asm/levels", func_L18_00428100);
INCLUDE_ASM("asm/levels", func_L18_00428138);

INCLUDE_ASM("asm/levels", func_L18_00428338);

/* Maps the class identifier this level tags an object with to its small integer. */
INCLUDE_ASM("asm/levels", func_L18_004283F0);
INCLUDE_ASM("asm/levels", func_L18_00428428);

/* Stores the measured halfword control for the one class that carries it, leaving
   the two silent classes alone and renewing the shared slot for anything else. */
extern void func_0026FEB8(void *);

extern u16 D_L18_001B1240 __attribute__((sda));

extern u8 D_L18_001B1360[];

void func_L18_004286D0(u8 *a0) {
    u8 *state = *(u8 **)(a0 + 0x68);
    switch (*(s16 *)(a0 + 0xAA)) {
        case 0xEC0:
            *(u16 *)(state + 0x392) = D_L18_001B1240;
            break;
        case 0xED4:
            break;
        case 0xEE1:
            break;
        default:
            func_0026FEB8(D_L18_001B1360);
            break;
    }
}

INCLUDE_ASM("asm/levels", func_L18_00428738);

INCLUDE_ASM("asm/levels", func_L18_00428AE8);

INCLUDE_ASM("asm/levels", func_L18_00428F08);

INCLUDE_ASM("asm/levels", func_L18_0042B008);

INCLUDE_ASM("asm/levels", func_L18_0043E760);

/* func_L18_0043E938 lives in the sibling small-data unit (L18_00427BE0-g8.c): its controls are read through $gp. */

INCLUDE_ASM("asm/levels", func_L18_0043E9B0);

INCLUDE_ASM("asm/levels", func_L18_0043EA90);

INCLUDE_ASM("asm/levels", func_L18_0043EDE8);

/* func_L18_0043EF40 is written with its sibling func_L18_0043F040 above. */

/* rotates the two 0xA0 blocks of the state's local frame into the shared matrices */
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern s32 D_L18_001B2348;

extern u8 D_L18_002BD9B0[];

extern s32 func_002A77E0(s32);

extern void func_00283098(void *, const void *, const void *);

extern void func_00282CC0(void *, void *, void *);

extern f32 func_00283B98(f32, f32);

extern f32 func_00283180(f32);

extern f32 func_00283198(f32);

void func_L18_0043F040(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 i;
    for (i = 0; i < 2; i++) {
        u8 *s = state + i * 0xA0;
        s32 j;
        for (j = 1; j >= 0; j--) {
            *(f32 *)(s + 0x40) = func_00283B98(*(f32 *)(s + 0x40), *(f32 *)(s + 0x60));
            *(f32 *)(s + 0x44) = func_00283B98(*(f32 *)(s + 0x44), *(f32 *)(s + 0x64));
            *(f32 *)(s + 0x48) = func_00283B98(*(f32 *)(s + 0x48), *(f32 *)(s + 0x68));
            *(f32 *)(s + 0x20) = func_00283180(*(f32 *)(s + 0x40)) * *(f32 *)(s + 0x80);
            *(f32 *)(s + 0x24) = func_00283198(*(f32 *)(s + 0x44)) * *(f32 *)(s + 0x84);
            *(f32 *)(s + 0x28) = -func_00283198(*(f32 *)(s + 0x48)) * *(f32 *)(s + 0x88);
            s += 0x10;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L18_0043EF40);
INCLUDE_ASM("asm/levels", func_L18_0043F428);

INCLUDE_ASM("asm/levels", func_L18_0043FFF8);

INCLUDE_ASM("asm/levels", func_L18_004401E0);

/* Store the supplied unsigned 32-bit word at owner byte offset 0xB 8. */
void func_L18_0046B1E0(int param_1, unsigned int param_2)
{
  *(unsigned int *)(param_1 + 0xb8) = param_2;
  return;
}
