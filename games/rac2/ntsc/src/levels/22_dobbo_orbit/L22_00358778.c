#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern s32 D_L22_001B2128 __attribute__((sda));
extern u8 D_L22_001A7A78 __attribute__((sda));
extern s32 D_L22_001A903C __attribute__((sda));

extern void func_31FBB8();
extern void func_3300E8();
extern s32 D_L22_0018B6A8 __attribute__((sda));

extern s32 func_2F36F0(s32);
extern void func_2F3720();
extern void func_2F3AB0(void *, void *, void *);
extern void func_2F3D60(void *, void *, f32);
extern void func_2F3A80(void *, void *, void *);
extern void func_2F9E20(s32, s32);
extern void func_32F330(s32, s32);
extern void func_3A2988(void *, f32);
extern void func_2F4788(s32, s32, f32);
extern f32 func_2F4AD0(f32, f32);
extern void func_3167E8(s32, s32, s32);
extern void func_362E98(s32, s32, s32);
extern void func_32AE80(s32, s32);

extern void func_L22_003E87B8(s32, s32, s32);
extern void func_L22_003FA1D0(s32, s32, s32);

INCLUDE_ASM("asm/levels", func_L22_00358778);

INCLUDE_ASM("asm/levels", func_L22_00385F40);

INCLUDE_ASM("asm/levels", func_L22_0038FBE0);

INCLUDE_ASM("asm/levels", func_L22_0038FFF8);

INCLUDE_ASM("asm/levels", func_L22_003955D8);

/* func_L22_003A6620 lives in the -g8 sibling file (it reaches a global through $gp). */

INCLUDE_ASM("asm/levels", func_L22_003B9450);

INCLUDE_ASM("asm/levels", func_L22_003CCA50);

INCLUDE_ASM("asm/levels", func_L22_003D0840);

INCLUDE_ASM("asm/levels", func_L22_003E87B8);

void func_L22_003E8830(s32 a0) {
    s32 x = *(u32 *)(a0 + 0x68);

    func_L22_003E87B8(a0, x, x);
    *(u8 *)(a0 + 0x30) = 0xFF;
}

INCLUDE_ASM("asm/levels", func_L22_003FA1D0);

void func_L22_003FA2A8(s32 a0) {
    s32 x = *(u32 *)(a0 + 0x68);

    func_L22_003FA1D0(a0, x, x);
    *(u8 *)(a0 + 0x30) = 0xFF;
}

INCLUDE_ASM("asm/levels", func_L22_003FD140);

INCLUDE_ASM("asm/levels", func_L22_0040C888);

INCLUDE_ASM("asm/levels", func_L22_0040D638);

void func_L22_0040DA80(s32 a0) {
    s32 s1 = a0;
    s32 s0 = *(u32 *)(s1 + 0x68);

    func_2F36F0(s0 + 0x258);
    func_2F3720(s0 + 0x25C);
    if (*(u16 *)(s1 + 0x96) <= 0xFFFE)
        *(u16 *)(s1 + 0x96) = *(u16 *)(s1 + 0x96) + 1;
    if (*(u8 *)(s0 + 0x211) == 0)
        func_3167E8(s1, 7, s0 + 0x210);
    func_2F4788(s0 + 0x220, 2, *(f32 *)(s0 + 0x26C));
    *(f32 *)(s0 + 0x26C) = func_2F4AD0(*(f32 *)(s0 + 0x26C), 0.41597018f);
}

INCLUDE_ASM("asm/levels", func_L22_0040DB20);

void func_L22_0040DC40(s32 a0, s32 a1, s32 a2, f32 f12) {
    u8 buf[0x10];
    u8 buf2[0x10];
    f32 v = f12;

    func_2F3AB0(buf, (void *)a1, (void *)(a0 + 0x10));
    v = v * 0.016666668f;
    func_2F3D60(buf, buf, v);
    if (a2 != 0) {
        func_2F3A80(buf2, (void *)(a0 + 0x10), buf);
        func_3A2988(buf2, 7.2f);
        func_2F3AB0(buf, buf2, (void *)(a0 + 0x10));
        func_2F3D60(buf, buf, v);
    }
    func_32F330(a0, a1);
    func_3300E8(a0, buf);
}

INCLUDE_ASM("asm/levels", func_L22_0040DD20);

INCLUDE_ASM("asm/levels", func_L22_0040DE00);

INCLUDE_ASM("asm/levels", func_L22_0040DF28);

INCLUDE_ASM("asm/levels", func_L22_0040E098);

INCLUDE_ASM("asm/levels", func_L22_0040F1C0);

INCLUDE_ASM("asm/levels", func_L22_004135F8);

INCLUDE_ASM("asm/levels", func_L22_00413B70);

INCLUDE_ASM("asm/levels", func_L22_00415A48);

INCLUDE_ASM("asm/levels", func_L22_0042ACA0);
