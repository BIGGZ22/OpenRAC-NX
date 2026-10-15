#include "include_asm.h"

typedef float f32;
typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;

extern f32 func_2F9748(f32, f32);
extern s32 func_32A4D0(u8 *, s32);
extern s32 func_31A718(u8 *);
extern s32 func_3248E0(u8 *, s32, s32);
extern void func_337400(s32, s32);
extern s32 func_2AF870(u8 *);
extern void func_327D00(f32 *, f32 *, f32, f32, f32, f32);
extern char D_00189EA0[];
extern f32 D_L13_001B2568 __attribute__((sda));
extern u32 D_L13_001B291C[] __attribute__((sda));
extern u8 D_L13_001B1E60[];
extern void func_2F8A20(u8 *, u8 *, f32);
extern u8 D_0019B2E8[];
extern s32 D_001A79F0 __attribute__((sda));
extern u8 *D_L13_001B2A0C __attribute__((sda));
extern u8 *D_L13_001B291C_2;
extern u8 D_00189E20[];

INCLUDE_ASM("asm/levels", func_L13_00306F38);

INCLUDE_ASM("asm/levels", func_L13_003383B8);

INCLUDE_ASM("asm/levels", func_L13_0035E890);

INCLUDE_ASM("asm/levels", func_L13_0038C328);

void func_L13_0038C470(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    switch (a0[0x20]) {
    case 0:
        a0[0x30] = 0xFF;
        func_3248E0(a0, 1, -1);
        break;
    case 1:
        if (func_32A4D0(D_00189EA0, ptr[0]) != 0) {
            func_337400(ptr[4], ptr[6]);
            func_3248E0(a0, 2, -1);
        }
        break;
    case 2:
        if (func_32A4D0(D_00189EA0, ptr[1]) != 0) {
            func_337400(ptr[5], ptr[7]);
            func_3248E0(a0, 1, -1);
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L13_0038C578);

INCLUDE_ASM("asm/levels", func_L13_0038C810);

INCLUDE_ASM("asm/levels", func_L13_00397600);

INCLUDE_ASM("asm/levels", func_L13_00397A18);

INCLUDE_ASM("asm/levels", func_L13_0039CFF8);

INCLUDE_ASM("asm/levels", func_L13_003D1778);

void func_L13_003E1218(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

void func_L13_003E1250(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

void func_L13_003E1288(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.029088825f);
}

void func_L13_003E12C0(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

void func_L13_003E12F8(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

INCLUDE_ASM("asm/levels", func_L13_003E8850);

INCLUDE_ASM("asm/levels", func_L13_003EFD58);

INCLUDE_ASM("asm/levels", func_L13_003FE420);

INCLUDE_ASM("asm/levels", func_L13_003FE708);

INCLUDE_ASM("asm/levels", func_L13_003FE970);

INCLUDE_ASM("asm/levels", func_L13_004067F8);

INCLUDE_ASM("asm/levels", func_L13_0041A398);

INCLUDE_ASM("asm/levels", func_L13_0041A550);

void func_L13_0041B260(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    u8 *base = D_00189EA0;
    if (func_32A4D0(base, ptr[0]) != 0) D_L13_001B2568 = 30.0f;
    if (func_32A4D0(base, ptr[1]) != 0) D_L13_001B2568 = 95.0f;
}

INCLUDE_ASM("asm/levels", func_L13_0041BDA8);

INCLUDE_ASM("asm/levels", func_L13_0041CD18);

INCLUDE_ASM("asm/levels", func_L13_0041E170);

INCLUDE_ASM("asm/levels", func_L13_0041F8B8);

INCLUDE_ASM("asm/levels", func_L13_00422CB8);

INCLUDE_ASM("asm/levels", func_L13_004230B0);

INCLUDE_ASM("asm/levels", func_L13_00424728);

INCLUDE_ASM("asm/levels", func_L13_004267B0);

/* func_L13_0042BE60: the catalogue entry starts 8 bytes before the body (a stray
   "addiu $sp,$sp,0x190 ; nop" tail). Not a function start — left as retail asm. */

INCLUDE_ASM("asm/levels", func_L13_0042BE60);

INCLUDE_ASM("asm/levels", func_L13_0042BEE0);

extern s32 func_2AF870(u8 *);
extern void func_327D00(f32 *, f32 *, f32, f32, f32, f32);
extern u8 D_L13_001B1E60[];

void func_L13_0042BFD8(u8 *a0) {
    u8 *ptr = *(u8 **)(a0 + 0x68);
    if (a0[0x20] == 0) {
        *(u16 *)(a0 + 0x34) |= 0x800;
        *(f32 *)(ptr + 0x8) = *(f32 *)(a0 + 0x18);
        *(f32 *)(ptr + 0x4) = *(f32 *)(a0 + 0x18) - 6.0f;
        a0[0x20] = 1;
    }
    if (a0[0x20] == 1) {
        u8 v = *((u8 *)D_L13_001B291C[0] + (*(u32 *)ptr << 8) + 0x20);
        if (v == 2) {
            a0[0x20] = v;
            func_2AF870(D_L13_001B1E60);
        }
    }
    if (a0[0x20] == 2) {
        func_327D00((f32 *)(a0 + 0x18), (f32 *)(ptr + 0xC), *(f32 *)(ptr + 0x4),
                    0.0016666667f, 0.0016666667f, 0.200000018f);
    }
}

INCLUDE_ASM("asm/levels", func_L13_0043E8C0);
