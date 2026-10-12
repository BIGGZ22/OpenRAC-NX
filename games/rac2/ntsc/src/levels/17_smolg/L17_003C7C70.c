#include "include_asm.h"

typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef short s16;
typedef char s8;
typedef float f32;

extern s32 D_L17_001FE880[];

extern void func_2EEAF8(s32);
extern void func_2EEB28(s32);
extern void func_320B58(s32, s32);
extern void func_327E68(s32, s32, s32, s32, f32);
extern s32 func_31B160(s32);
extern f32 func_2EF0B0(s32, s32);

INCLUDE_ASM("asm/levels", func_L17_003C7C70);

INCLUDE_ASM("asm/levels", func_L17_003C8248);

INCLUDE_ASM("asm/levels", func_L17_003CC6A8);

INCLUDE_ASM("asm/levels", func_L17_003CC798);

INCLUDE_ASM("asm/levels", func_L17_003CC850);

INCLUDE_ASM("asm/levels", func_L17_003D0620);

INCLUDE_ASM("asm/levels", func_L17_003E3148);

INCLUDE_ASM("asm/levels", func_L17_003E31E8);

INCLUDE_ASM("asm/levels", func_L17_003E41A8);

INCLUDE_ASM("asm/levels", func_L17_003E4560);

void func_L17_003E47E8(s32 a0) {
    s32 s1 = *(s32 *)(a0 + 0x68);

    func_320B58(a0, s1 + 0x50);
    func_2EEAF8(s1 + 0x3FC);
    func_2EEB28(s1 + 0x402);
    if (*(u16 *)(a0 + 0x96) < 0xFFFF) {
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    }
    func_327E68(a0, s1 + 0x350, 1, 1, 2.5f);
}

INCLUDE_ASM("asm/levels", func_L17_003E4860);

INCLUDE_ASM("asm/levels", func_L17_003E48E0);

INCLUDE_ASM("asm/levels", func_L17_003E6DE8);

INCLUDE_ASM("asm/levels", func_L17_003E6F20);

s32 func_L17_003E6F58(s32 a0) {
    s32 s0;
    s32 s1 = D_L17_001FE880[*(s32 *)(*(s32 *)(a0 + 0x68) + 0x684)];
    s32 s4 = *(s32 *)s1;
    s32 s2 = 0;

    do {
        s32 off;

        s0 = func_31B160(s4);
        off = (s0 << 4) + 0x10;
        if (func_2EF0B0(a0 + 0x10, s1 + off) > 3.0f) {
            return s0;
        }
    } while (++s2 < 5);
    return s0;
}

INCLUDE_ASM("asm/levels", func_L17_003E7010);

INCLUDE_ASM("asm/levels", func_L17_003E7068);

INCLUDE_ASM("asm/levels", func_L17_003E8A58);

INCLUDE_ASM("asm/levels", func_L17_00417DB0);

INCLUDE_ASM("asm/levels", func_L17_00418A68);

INCLUDE_ASM("asm/levels", func_L17_00418D10);

INCLUDE_ASM("asm/levels", func_L17_0041A4C0);

INCLUDE_ASM("asm/levels", func_L17_0041E208);

INCLUDE_ASM("asm/levels", func_L17_0041EAA8);

INCLUDE_ASM("asm/levels", func_L17_0041ED58);

INCLUDE_ASM("asm/levels", func_L17_0041F180);

INCLUDE_ASM("asm/levels", func_L17_0041F478);

INCLUDE_ASM("asm/levels", func_L17_0041F8F0);

INCLUDE_ASM("asm/levels", func_L17_00420758);

INCLUDE_ASM("asm/levels", func_L17_00420A50);

INCLUDE_ASM("asm/levels", func_L17_00420E08);
