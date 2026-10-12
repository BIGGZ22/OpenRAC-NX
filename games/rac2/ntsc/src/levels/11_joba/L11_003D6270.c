#include "common.h"

extern void func_002A7750(u8 *arg0, s32 arg1, s32 arg2);

extern u8 *func_002A9640(s32 arg0, s32 arg1, s32 arg2);

extern void func_002A7470(u8 *arg0, s32 arg1, s32 arg2, s32 arg3);

extern void func_002A05A8(u8 *arg0);

extern s32 func_002E59B0(u8 *arg0, s32 arg1);

extern s32 func_002E5CF8(s32 arg0, s32 arg1, u8 *arg2);

extern void func_002E59F8(s32 arg0);

extern s32 func_00283CF0(f32 arg0);

extern void func_002A04B8(u8 *arg0, u8 *arg1, u8 *arg2, u8 *arg3);

extern void func_002A93B0(u8 *arg0);

extern f32 func_00282E48(u8 *arg0, u8 *arg1);

extern void func_L11_003F1D18();

extern void func_L11_003F1DB0(u8 *arg0);

extern void func_L11_003F1F48(u8 *arg0);

extern void func_L11_003F2A30(u8 *arg0);

extern void func_L11_003F2C28(u8 *arg0);

extern s32 func_L11_003F2E00(u8 *arg0);

extern void func_L11_003F3098(u8 *arg0);

extern void func_L11_003F31C0(u8 *arg0);

extern u8 D_L11_00188660[];

extern u8 D_L11_001BA8C0[];

INCLUDE_ASM("asm/levels", func_L11_003D6270);

INCLUDE_ASM("asm/levels", func_L11_003D90C0);

INCLUDE_ASM("asm/levels", func_L11_003DCEB0);

INCLUDE_ASM("asm/levels", func_L11_003DD068);

void func_L11_003F1C68(u8 *arg0) {
    u8 *s0 = arg0;
    if (*(u8 *)(s0 + 0x20) == 0) {
        func_L11_003F1D18();
    } else {
        if (func_L11_003F2E00(s0) != 0)
            return;
        if (*(u8 *)(s0 + 0x31) != 0
            && func_00282E48(s0 + 0x10, D_L11_001BA8C0) < 48.0f) {
            func_002A93B0(s0);
            *(u8 *)(s0 + 0x6F) = 0x2D;
        }
        func_L11_003F1DB0(s0);
        func_L11_003F1F48(s0);
        func_L11_003F2A30(s0);
        func_L11_003F2C28(s0);
    }
}

void func_L11_003F1D18(u8 *arg0) {
    u8 *s1 = arg0;
    u8 *s0 = *(u8 **)(s1 + 0x68);
    *(u8 *)(s0 + 0x48) = func_00283CF0(15.0f);
    *(u8 *)(s0 + 0x4A) = func_00283CF0(16.0f);
    func_002A04B8(s1, s0 + 0x280, s0 + 0x284, s0 + 0x288);
    *(u8 *)(s0 + 0x28) = 3;
    *(f32 *)(s0 + 0x30) = 1.5f;
    *(u8 *)(s0 + 0x29) = 1;
    *(u16 *)(s1 + 0x34) |= 0x100;
    func_L11_003F3098(s1);
    *(s32 *)(s0 + 0x290) = -1;
}

INCLUDE_ASM("asm/levels", func_L11_003F1DB0);

INCLUDE_ASM("asm/levels", func_L11_003F1F48);

INCLUDE_ASM("asm/levels", func_L11_003F2A30);

INCLUDE_ASM("asm/levels", func_L11_003F2C28);


INCLUDE_ASM("asm/levels", func_L11_003F2E00);

void func_L11_003F2D18(u8 *arg0) {
    func_002A7750(arg0, 4, -1);
}

void func_L11_003F2DC8(u8 *arg0) {
    u8 *v0 = arg0;
    s32 a3 = *(s32 *)(v0 + 0x68);
    u8 t0 = *(u8 *)(v0 + 0x20);
    u8 v1 = *(u8 *)(v0 + 0x95);
    *(u8 *)(v0 + 0x94) = t0;
    *(u16 *)(a3 + 0x294) = v1;
    func_002A7750(arg0, 3, -1);
}

/* Reports the signed area of the triangle (arg0, arg1, arg2). */
f32 func_L11_003F2F00(f32 *arg0, f32 *arg1, f32 *arg2) {
    return (arg0[0] - arg2[0]) * (arg1[1] - arg2[1])
         - (arg0[1] - arg2[1]) * (arg1[0] - arg2[0]);
}

INCLUDE_ASM("asm/levels", func_L11_003F2E38);

void func_L11_003F3098(u8 *arg0) {
    func_L11_003F31C0(arg0);
    func_002A7470(arg0, 0, 0, 3);
    func_002A05A8(arg0);
    func_002A7750(arg0, 1, 0);
    *(u8 *)(arg0 + 0x31) = 0;
    *(s32 *)(arg0 + 0x98) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 1) & 0xEFFF;
}

void func_L11_003F3108(u8 *arg0) {
    s32 i = 1;
    u16 *v0 = (u16 *)(*(u8 **)(arg0 + 0x68) + 0x278);
    do {
        v0[-2] = 0;
        v0[0] = 1;
        i--;
        v0++;
    } while (i >= 0);
}

void func_L11_003F3140(u8 *arg0) {
    s32 i = 1;
    u8 *s0 = arg0;
    u8 *s1 = *(u8 **)(s0 + 0x68);
    u16 *v0 = (u16 *)(s1 + 0x278);
    do {
        v0[-2] = 0;
        v0[0] = 2;
        i--;
        v0++;
    } while (i >= 0);
    if (func_002E59B0(s0, *(s32 *)(s1 + 0x290)) == 0) {
        *(s32 *)(s1 + 0x290) = func_002E5CF8(2, 4, s0);
    }
}

void func_L11_003F31C0(u8 *arg0) {
    s32 i = 1;
    u8 *a2 = arg0;
    u8 *s0 = *(u8 **)(a2 + 0x68);
    u16 *v0 = (u16 *)(s0 + 0x278);
    s32 a1;
    do {
        v0[-2] = 0;
        v0[0] = 0;
        i--;
        v0++;
    } while (i >= 0);
    a1 = *(s32 *)(s0 + 0x290);
    if (a1 != -1) {
        u8 *v1 = D_L11_00188660 + a1 * 0x70;
        if (*(u8 **)(v1 + 0x88) == a2 && *(u8 *)(v1 + 0x74) != 0) {
            func_002E59F8(a1);
        }
    }
    *(s32 *)(s0 + 0x290) = -1;
}

INCLUDE_ASM("asm/levels", func_L11_003F2F38);

INCLUDE_ASM("asm/levels", func_L11_003F3258);

INCLUDE_ASM("asm/levels", func_L11_003F33E0);

INCLUDE_ASM("asm/levels", func_L11_003F3CB8);

INCLUDE_ASM("asm/levels", func_L11_003F4060);

INCLUDE_ASM("asm/levels", func_L11_003F4490);

INCLUDE_ASM("asm/levels", func_L11_003F4A58);

INCLUDE_ASM("asm/levels", func_L11_003F4F30);

INCLUDE_ASM("asm/levels", func_L11_003F7B70);

INCLUDE_ASM("asm/levels", func_L11_003F7F58);

INCLUDE_ASM("asm/levels", func_L11_003F8150);

INCLUDE_ASM("asm/levels", func_L11_003F8510);

INCLUDE_ASM("asm/levels", func_L11_003F9F60);

INCLUDE_ASM("asm/levels", func_L11_00411D40);

INCLUDE_ASM("asm/levels", func_L11_00412B20);

INCLUDE_ASM("asm/levels", func_L11_00415E70);

INCLUDE_ASM("asm/levels", func_L11_00416658);

INCLUDE_ASM("asm/levels", func_L11_004169E0);

INCLUDE_ASM("asm/levels", func_L11_00416D70);

INCLUDE_ASM("asm/levels", func_L11_00417340);

/* Reports whether the helper found the measured resource and updates its timer. */
s32 func_L11_00420C38(u8 *arg0) {
    u8 *s0 = *(u8 **)(arg0 + 0x68);
    u8 *v0 = func_002A9640(*(s32 *)(s0 + 0x280), 0x10000, 0);
    if (v0 != 0) {
        *(f32 *)(s0 + 0x20) = *(f32 *)(s0 + 0x20) - *(f32 *)(v0 + 0x2C);
        return 1;
    }
    return 0;
}
