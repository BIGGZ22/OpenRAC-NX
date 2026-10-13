#include "include_asm.h"

typedef signed int s32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef float f32;

extern u32 *func_0037BA88(u8 *arg0);
extern s32 func_003351C0(s32 mode);
extern void func_003737B8(s32 a, s32 b, u8 *c, s32 d);
extern void func_0037BD88(u8 *arg0, u32 flags, s32 a, s32 b, s32 c, s32 d, s32 e,
                          f32 f0, f32 f1, f32 f2, f32 f3);


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

INCLUDE_ASM("asm/levels", func_L10_003F6110);

INCLUDE_ASM("asm/levels", func_L10_003F61C8);

INCLUDE_ASM("asm/levels", func_L10_00415A50);

INCLUDE_ASM("asm/levels", func_L10_004166E8);

INCLUDE_ASM("asm/levels", func_L10_00417040);

INCLUDE_ASM("asm/levels", func_L10_004170F8);

INCLUDE_ASM("asm/levels", func_L10_00417440);

INCLUDE_ASM("asm/levels", func_L10_004174C8);

INCLUDE_ASM("asm/levels", func_L10_004175C8);

INCLUDE_ASM("asm/levels", func_L10_004176A0);

INCLUDE_ASM("asm/levels", func_L10_004176B0);

INCLUDE_ASM("asm/levels", func_L10_004176C0);

INCLUDE_ASM("asm/levels", func_L10_004177E8);

INCLUDE_ASM("asm/levels", func_L10_00417A38);

INCLUDE_ASM("asm/levels", func_L10_00417C40);

INCLUDE_ASM("asm/levels", func_L10_00417CE0);

INCLUDE_ASM("asm/levels", func_L10_00417F30);

INCLUDE_ASM("asm/levels", func_L10_00418088);

INCLUDE_ASM("asm/levels", func_L10_00418400);

INCLUDE_ASM("asm/levels", func_L10_004184F0);

INCLUDE_ASM("asm/levels", func_L10_00418588);

INCLUDE_ASM("asm/levels", func_L10_00418768);

INCLUDE_ASM("asm/levels", func_L10_00418968);

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

INCLUDE_ASM("asm/levels", func_L10_0041ED50);

INCLUDE_ASM("asm/levels", func_L10_0041EE00);

INCLUDE_ASM("asm/levels", func_L10_00421288);

INCLUDE_ASM("asm/levels", func_L10_00421790);

INCLUDE_ASM("asm/levels", func_L10_004227A0);

INCLUDE_ASM("asm/levels", func_L10_00423500);

INCLUDE_ASM("asm/levels", func_L10_00423788);

INCLUDE_ASM("asm/levels", func_L10_004239A0);

INCLUDE_ASM("asm/levels", func_L10_00423DC8);
