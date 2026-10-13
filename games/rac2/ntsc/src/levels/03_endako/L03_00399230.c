#include "common.h"

extern void func_002A7750(char *a0, s32 a1, s32 a2);
extern void func_324168(s32 a0, s32 a1);
extern void func_30D710(s32 a0, s32 a1);
extern void func_32C010();
extern void func_32C328(char *a0, char *a1, s32 a2);
extern void *func_31DD78(char *a0, s32 a1, s32 a2);
extern s32  func_32AB10(char *a0);
extern void func_360A80(s32 a0, s32 a1, char *a2);
extern void func_311758(void *a0);
extern void func_L03_003D11E0(char *a0);

INCLUDE_ASM("asm/levels", func_L03_00399230);

INCLUDE_ASM("asm/levels", func_L03_003C5FE8);

INCLUDE_ASM("asm/levels", func_L03_003C6208);

INCLUDE_ASM("asm/levels", func_L03_003C6A28);

INCLUDE_ASM("asm/levels", func_L03_003C6E50);

INCLUDE_ASM("asm/levels", func_L03_003C7018);

INCLUDE_ASM("asm/levels", func_L03_003C72A8);

void func_L03_003C7A40(char *a0)
{
    char *v1 = *(char **)(a0 + 0x68);

    *(u16 *)(v1 + 0x106) = 0;
    *(s32 *)(v1 + 0x100) = 0;
    *(u16 *)(a0 + 0x34) |= 0x400;
    func_002A7750(a0, 2, -1);
    *(s32 *)(a0 + 0x9C) = 0;
}

void func_L03_003C7A88(char *a0)
{
    char *v0 = *(char **)(a0 + 0x68);

    *(u16 *)(v0 + 0x106) = 1;
    *(s32 *)(a0 + 0x9C) = 1;
}

void func_L03_003C7AA0(char *a0, char *a1)
{
    char *a2 = *(char **)(a0 + 0x68);

    qcopy(a1, a0 + 0x10);
    *(f32 *)(a1 + 8) = *(f32 *)(a2 + 0xF8);
}

INCLUDE_ASM("asm/levels", func_L03_003C7AC0);

INCLUDE_ASM("asm/levels", func_L03_003C7E20);

INCLUDE_ASM("asm/levels", func_L03_003C85F8);

void func_L03_003C8660(char *a0, char *a1)
{
    char *v1 = *(char **)(a0 + 0x68);

    *(s32 *)(v1 + 0xAC) = 0;
    *(s32 *)(v1 + 0xA0) = 0;
    qcopy(v1 + 0x80, a1);
    func_002A7750(a0, 3, -1);
}

INCLUDE_ASM("asm/levels", func_L03_003C8698);

/* func_L03_003CA938 lives in the -G8 sibling L03_00399230-g8.c. */

INCLUDE_ASM("asm/levels", func_L03_003CACA8);

INCLUDE_ASM("asm/levels", func_L03_003CAD78);

INCLUDE_ASM("asm/levels", func_L03_003CB398);

INCLUDE_ASM("asm/levels", func_L03_003CBC40);

INCLUDE_ASM("asm/levels", func_L03_003CBD70);

INCLUDE_ASM("asm/levels", func_L03_003CBE58);

INCLUDE_ASM("asm/levels", func_L03_003CF0C0);

s32 func_L03_003CF7B8(u8 *a0)
{
    return (u8)(a0[0x20] - 2) < 3;
}

INCLUDE_ASM("asm/levels", func_L03_003CF7C8);

void func_L03_003CFB30(char *a0)
{
    char *moby = *(char **)(a0 + 0x68);

    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(s32 *)(moby + 0x5C) |= 1;
        func_32C010(a0, moby + 0x60, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        func_002A7750(a0, 1, -1);
        break;
    case 1:
        func_32C328(a0, moby + 0x60, 0);
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L03_003D0300);

void func_L03_003D1100(char *a0)
{
    char *a3 = *(char **)(a0 + 0x68);

    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(s32 *)(a3 + 0x5C) |= 8;
        func_002A7750(a0, 1, -1);
        break;
    case 1: {
        char *s1 = (char *)func_31DD78(a0, 0x10000, 0);
        s32 v;

        if (func_32AB10(a0) != 0
            || (s1 != 0 && (v = *(s32 *)(s1 + 0x20)) != 0
                && *(s16 *)(v + 0xAA) == 0xB1B)) {
            func_L03_003D11E0(a0);
            func_360A80(0, 0, a0);
            func_311758(a0);
        } else {
            *(u8 *)(a0 + 0xA8) = 0xFF;
        }
        break;
    }
    }
}

INCLUDE_ASM("asm/levels", func_L03_003D11E0);

INCLUDE_ASM("asm/levels", func_L03_003D1548);

INCLUDE_ASM("asm/levels", func_L03_003D18F8);

INCLUDE_ASM("asm/levels", func_L03_003D19C8);

void func_L03_003D1FB0(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);

    func_324168(*(s32 *)(s0 + 0x18), *(s32 *)(s0 + 0x1C));
    func_30D710(0, -1);
    *(u16 *)(*(s32 *)(s0 + 0x2C) + 0x34) |= 0x41;
    *(u16 *)(*(s32 *)(s0 + 0x2C) + 0x34) &= ~0x1000;
    *(s32 *)(*(s32 *)(s0 + 0x2C) + 0x98) = 0;
    *(u16 *)(*(s32 *)(s0 + 0x2C) + 0x96) = 0;
}

INCLUDE_ASM("asm/levels", func_L03_003D2018);

INCLUDE_ASM("asm/levels", func_L03_003D20F8);

INCLUDE_ASM("asm/levels", func_L03_003D21F8);

INCLUDE_ASM("asm/levels", func_L03_003D22C8);

INCLUDE_ASM("asm/levels", func_L03_003D2518);

INCLUDE_ASM("asm/levels", func_L03_003D5788);

INCLUDE_ASM("asm/levels", func_L03_003E18D8);

INCLUDE_ASM("asm/levels", func_L03_003E1C88);
