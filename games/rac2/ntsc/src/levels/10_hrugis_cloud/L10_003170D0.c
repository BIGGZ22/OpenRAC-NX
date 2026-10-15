#include "include_asm.h"

typedef signed int s32;
typedef signed short s16;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern u32 *func_0037BA88(u8 *arg0);
extern s32 func_003351C0(s32 mode);
extern s32 func_003737B8(s32 a, s32 b, u8 *c, s32 d);
extern void func_0037BD88(u8 *arg0, u32 flags, s32 a, s32 b, s32 c, s32 d, s32 e,
                          f32 f0, f32 f1, f32 f2, f32 f3);
extern void func_0037CCB0(u8 *arg0, s32 a1);
extern void func_0032CBD0(u8 *arg0, s32 *a1, s32 *a2, s32 *a3);
extern void func_0032CBA0(u8 *arg0, s32 a1, s32 a2, s32 a3);
extern f32 func_00335320(void);
extern void func_L10_00417F30(u8 *arg0);
extern void func_L10_00418088(u8 *arg0);
extern u8 *func_0037B670(u8 *arg0);
extern void func_0037BC48(u8 *arg0, s32 a1, s32 a2, s32 a3);
extern s32 func_003062A0(u8 *a0);
extern s32 func_0037D390(u8 *arg0, s32 a1, s32 a2, s32 a3);
extern s32 func_00335200(s32 a0, s32 a1);
extern void func_0032BD98(s32 a0, u8 *a1);
extern void func_0032AF78(s32 a0);
extern void func_00308BB8(f32 *a0, u8 *a1, f32 f12);
extern f32 func_00309138(f32 f12);
extern f32 func_00309120(f32 f12);
extern void func_003098F8(f32 *a0, f32 *a1);
extern void func_003096D8(u8 *a0, f32 *a1, u8 *a2);
extern void func_00308B30(u8 *a0, u8 *a1, u8 *a2);
extern void func_00335258(f32 f12, f32 f13);
extern void func_0033D090(u8 *a0, f32 f12, f32 f13, f32 f14);
extern void func_00373410(s32 a0);
extern void func_L10_004184F0(u8 *arg0);
extern void func_L10_00418588(u8 *arg0);
extern void func_L10_004189D8(u8 *arg0);
extern void func_L10_00418AD8(u8 *arg0);
extern void func_L10_00418BA0(u8 *arg0);
extern void func_00308AF8(u8 *a0);
extern void func_0037DFB8(u8 *a0);
extern void func_0037D7A0(u8 *a0, s32 a1, f32 f12);
extern s32 func_0037D6E0(u8 *a0);
extern void func_0037D528(u8 *a0);

extern u8 *D_L10_0020B1E0[];
extern u8 *D_001B251C;
extern u8 D_L10_00265BA0[];
extern u8 D_L10_00188660[];
extern u8 D_L10_00221CD0[];
extern s32 D_001B0F44 __attribute__((sda));
extern s32 D_001B0F48 __attribute__((sda));


INCLUDE_ASM("asm/levels", func_L10_003170D0);

INCLUDE_ASM("asm/levels", func_L10_00317528);

#define NOT_SDA __attribute__((section(".data")))

extern void *r1_D_0012F86C_0037CC20 NOT_SDA;

/* rac1-decomp src/core/001138A8.c: func_001138A8 */
int func_L10_0037CC20(void) {
    return (int)r1_D_0012F86C_0037CC20;
}

INCLUDE_ASM("asm/levels", func_L10_003ABDF8);

INCLUDE_ASM("asm/levels", func_L10_003F5EF8);

void func_L10_003F6110(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);
    u8 *s2 = (u8 *)func_0037BA88(arg0);

    *(s32 *)(s2 + 0x148) = 2;
    func_0037CCB0(arg0, 1);
    *(f32 *)(s1 + 0x180) = 2.0f;
    arg0[0x20] = 1;
    arg0[0x30] = 0xFF;
    *(u16 *)(arg0 + 0x32) = 0x1FF;
    *(u16 *)(arg0 + 0x34) = *(u16 *)(arg0 + 0x34) | 0x100;
    *(f32 *)(arg0 + 0x2C) = *(f32 *)(*(u8 **)(arg0 + 0x24) + 0x24) * 4.1f;
    *(s32 *)(s2 + 0x14C) = 0;
    *(u16 *)(arg0 + 0x34) = *(u16 *)(arg0 + 0x34) & 0xEFFF;
}

INCLUDE_ASM("asm/levels", func_L10_003F61C8);

INCLUDE_ASM("asm/levels", func_L10_00415A50);

INCLUDE_ASM("asm/levels", func_L10_004166E8);

void func_L10_00417040(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);

    *(s32 *)((u8 *)func_0037BA88(arg0) + 0x148) = 0x1E;
    func_0037CCB0(arg0, 1);
    *(f32 *)(s1 + 0x180) = 30.0f;
    arg0[0x20] = 1;
    arg0[0x30] = 0xFF;
    *(u16 *)(arg0 + 0x32) = 0x1FF;
    *(f32 *)(s1 + 0x2BC) = func_00335320();
    *(f32 *)(s1 + 0x2B8) = *(f32 *)(arg0 + 0x18);
    *(s32 *)(s1 + 0x2C0) = 0;
    *(s32 *)(s1 + 0x2C4) = 0;
    *(s32 *)(s1 + 0x2C8) = 0;
    *(f32 *)(arg0 + 0x2C) = *(f32 *)(*(u8 **)(arg0 + 0x24) + 0x24) * 6.4f;
    *(s32 *)(s1 + 0x2E4) = 0x21C;
    *(s32 *)(s1 + 0x2D0) = 0;
    *(s32 *)(s1 + 0x2CC) = 0;
    *(u16 *)(arg0 + 0x7C) = 0xFF;
}

INCLUDE_ASM("asm/levels", func_L10_004170F8);

INCLUDE_ASM("asm/levels", func_L10_00417440);

INCLUDE_ASM("asm/levels", func_L10_004174C8);

void func_L10_004175C8(u8 *arg0) {
    u8 *s = *(u8 **)(arg0 + 0x68);

    switch (arg0[0x20]) {
    case 1:
        *(f32 *)(s + 0x2CC) = *(f32 *)(s + 0x2CC) + 3.87851e-05f;
        *(f32 *)(s + 0x2D0) = *(f32 *)(s + 0x2D0) + 3.87851e-05f;
        if (*(f32 *)(s + 0x2CC) > 0.023271061f)
            *(f32 *)(s + 0x2CC) = 0.023271061f;
        if (*(f32 *)(s + 0x2D0) > 0.011635531f)
            *(f32 *)(s + 0x2D0) = 0.011635531f;
        break;
    case 2:
        *(f32 *)(s + 0x2CC) = *(f32 *)(s + 0x2CC) - 5.8177648e-05f;
        *(f32 *)(s + 0x2D0) = *(f32 *)(s + 0x2D0) - 5.8177648e-05f;
        if (*(f32 *)(s + 0x2CC) < 0.0f)
            *(f32 *)(s + 0x2CC) = 0.0f;
        if (*(f32 *)(s + 0x2D0) < 0.0f)
            *(f32 *)(s + 0x2D0) = 0.0f;
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L10_004176C0);

INCLUDE_ASM("asm/levels", func_L10_004177E8);

INCLUDE_ASM("asm/levels", func_L10_00417A38);

void func_L10_00417C40(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);
    u32 st = arg0[0x20];
    s32 a, b, c;
    u8 *v;

    switch (st) {
    case 0:
        arg0[0x20] = 1;
        /* fall through */
    case 1:
        func_0032CBD0(*(u8 **)(s1 + 0x10), &a, &b, &c);
        func_0032CBA0(arg0, a, b, c);
        break;
    default:
        return;
    }
    func_L10_00417F30(arg0);
    v = (u8 *)func_0037BA88(*(u8 **)(s1 + 0x10));
    if (*(s32 *)(v + 0x120) != 0)
        func_L10_00418088(arg0);
}

INCLUDE_ASM("asm/levels", func_L10_00417CE0);

INCLUDE_ASM("asm/levels", func_L10_00417F30);

INCLUDE_ASM("asm/levels", func_L10_00418088);

INCLUDE_ASM("asm/levels", func_L10_00418400);

INCLUDE_ASM("asm/levels", func_L10_004184F0);

INCLUDE_ASM("asm/levels", func_L10_00418588);

INCLUDE_ASM("asm/levels", func_L10_00418768);


void func_L10_00418968(u8 *arg0) {
    u8 *s0 = *(u8 **)(arg0 + 0x68);
    s32 id = *(s32 *)(s0 + 0x26C);

    if (id != -1) {
        u8 *p = D_L10_00188660 + id * 0x70;

        if (*(u8 **)(p + 0x88) == arg0 && *(u8 *)(p + 0x74) != 0)
            func_00373410(id);
    }
    *(s32 *)(s0 + 0x26C) = -1;
}

INCLUDE_ASM("asm/levels", func_L10_004189D8);

/* arms one timed effect slot and records it in the state word, skipping out while the
   object is inactive */
void func_L10_00418AD8(u8 *arg0) {
    u8 *state = *(u8 **)(arg0 + 0x68);
    u8 *p = (u8 *)func_0037BA88(arg0);
    if (p == 0) return;
    if (*(s32 *)(p + 0x120) == 0) return;
    if (*(s32 *)(state + 0x268) != 0) return;
    func_0037BD88(arg0, 0x60000080, 1, 0, 0, -1, 0, 0.8f, 2.0f, 1.0f, 1.3f);
    if (func_003351C0(3) != 0) {
        func_003737B8(1, 0, arg0, 0xF8E);
    }
    *(s32 *)(state + 0x268) = 0x1E;
}

INCLUDE_ASM("asm/levels", func_L10_00418BA0);

INCLUDE_ASM("asm/levels", func_L10_00419188);

INCLUDE_ASM("asm/levels", func_L10_0041EC10);

void func_L10_0041ED50(u8 *arg0) {
    u8 *s3 = *(u8 **)(arg0 + 0x68);
    u8 *s1 = (u8 *)func_0037BA88(arg0);

    *(s32 *)(s1 + 0x148) = 1;
    func_0037CCB0(arg0, 1);
    *(s32 *)(s1 + 0x14C) = 0;
    *(f32 *)(s3 + 0x180) = 1.0f;
    arg0[0x30] = 0xFF;
    arg0[0x20] = 1;
    *(u16 *)(arg0 + 0x32) = 0x1FF;
    *(u16 *)(arg0 + 0x34) |= 0x100;
    *(f32 *)(arg0 + 0x2C) = *(f32 *)(*(u8 **)(arg0 + 0x24) + 0x24) * 4.1f;
}

INCLUDE_ASM("asm/levels", func_L10_0041EE00);

INCLUDE_ASM("asm/levels", func_L10_00421288);

INCLUDE_ASM("asm/levels", func_L10_00421790);

INCLUDE_ASM("asm/levels", func_L10_004227A0);

INCLUDE_ASM("asm/levels", func_L10_00423500);

INCLUDE_ASM("asm/levels", func_L10_00423788);

INCLUDE_ASM("asm/levels", func_L10_004239A0);

INCLUDE_ASM("asm/levels", func_L10_00423DC8);
