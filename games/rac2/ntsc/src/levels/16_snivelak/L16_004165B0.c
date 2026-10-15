#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

INCLUDE_ASM("asm/levels", func_L16_004165B0);

extern void func_002E74B8(char *, char *, char *);
extern void func_002E7788(char *, char *, f32);
extern void func_003E9FF0(char *, char *, char *, f32);
extern void func_00357288(s32, s32, char *);
extern f32 func_002E73F8(f32);
extern f32 func_00313F98(f32, f32);
extern f32 func_00313F38(f32, f32);
extern void func_002D4CC8(char *, s32, s32, s32, char *, char *, char *, s32, f32, f32);
extern void func_003140C0(char *, f32, f32);
extern char D_001B9B00[];

/* lays the object's ten measurement points out along a ring and asks the engine to arm them */
void func_L16_00416D08(char *arg0) {
    char buf0[0x10];
    char buf1[0x10];
    f32 f20;
    f32 f30;
    s16 h;
    s32 i;

    func_002E74B8(buf0, D_001B9B00, arg0 + 0x10);
    func_002E7788(buf0, buf0, 1.0f);
    func_003E9FF0(arg0, arg0 + 0x10, buf0, 0.0f);
    func_00357288(0, 0, arg0);
    for (i = 0; i < 5; i++) {
        func_003140C0(buf1, 0.0833333358168602f, 0.25f);
        *(f32 *)(buf1 + 8) = func_002E73F8(*(f32 *)(buf1 + 8));
        h = *(s16 *)(arg0 + 0xAA);
        f20 = func_00313F98(90.0f, 180.0f);
        f30 = func_00313F38(10.0f, 20.0f);
        func_002D4CC8(arg0, 2, h, i, arg0 + 0x10, arg0 + 0xF0, buf1, 0xFF, f20, f30);
    }
}

/* the horizontal speed the object carries in its data block */
f32 func_L16_00416EB8(char *arg0) {
    return *(f32 *)(*(char **)(arg0 + 0x68) + 0x78);
}

INCLUDE_ASM("asm/levels", func_L16_00416EC8);

INCLUDE_ASM("asm/levels", func_L16_00416FB0);

INCLUDE_ASM("asm/levels", func_L16_00417210);

/* the model index the object's data block points at */
s32 func_L16_00417788(char *arg0) {
    char *p = *(char **)(arg0 + 0x68);
    return *(s32 *)(p + 0x60);
}

INCLUDE_ASM("asm/levels", func_L16_00417798);

INCLUDE_ASM("asm/levels", func_L16_00417990);

INCLUDE_ASM("asm/levels", func_L16_00417DB0);

INCLUDE_ASM("asm/levels", func_L16_004182A8);

INCLUDE_ASM("asm/levels", func_L16_004185C0);

INCLUDE_ASM("asm/levels", func_L16_004189A0);

INCLUDE_ASM("asm/levels", func_L16_00418E38);

INCLUDE_ASM("asm/levels", func_L16_00419140);

INCLUDE_ASM("asm/levels", func_L16_004192A8);

extern char D_00189E20[];
extern void func_00315728(char *, char *, s32, char *, f32);
extern void func_00315948(char *, char *);

/* next frame: arms the object once the shared render state names it, then hands its box to the collision pass */
void func_L16_0041C6A0(char *arg0) {
    char buf[0x30];
    char v[0x10];
    char *q;
    char *p;

    if (*(u8 *)(arg0 + 0x20) == 0) {
        *(u8 *)(arg0 + 0x20) = 1;
        *(u8 *)(arg0 + 0x31) = 0;
        *(u16 *)(arg0 + 0x34) |= 1;
    }
    if (*(u8 *)(arg0 + 0x20) != 1)
        return;
    q = D_00189E20;
    p = *(char **)(q + 0x33C);
    if (p == 0 || p != arg0)
        return;
    if (*(s32 *)(q + 0x340) == 0)
        return;
    *(s32 *)(v + 0) = 0;
    *(f32 *)(v + 4) = 1.0f;
    *(s32 *)(v + 8) = 0;
    *(f32 *)(v + 0xC) = 1.0f;
    func_002E7788(v, v, 10.0f);
    func_00315728(buf, arg0, 1, v, (f32)(*(s32 *)0x001A7A00 * 200));
    *(u16 *)(buf + 0x1A) = *(u16 *)(arg0 + 0xAA);
    func_00315948(*(char **)(q + 0x2290), buf);
}

INCLUDE_ASM("asm/levels", func_L16_0041C7A0);

INCLUDE_ASM("asm/levels", func_L16_0041C990);

INCLUDE_ASM("asm/levels", func_L16_0041CB30);

INCLUDE_ASM("asm/levels", func_L16_0041CF80);

INCLUDE_ASM("asm/levels", func_L16_0041E370);

INCLUDE_ASM("asm/levels", func_L16_0041EBE8);

INCLUDE_ASM("asm/levels", func_L16_0041ECF8);

INCLUDE_ASM("asm/levels", func_L16_0041EE90);

extern void func_002E74B8(char *, char *, char *);
extern s32 func_00313EE0(s32, s32);
extern s32 func_00313EA0(s32);
extern s32 func_003320E0(char *, char *, u32, u32, f32, u32, u32);
extern s32 func_002E8648(f32);

/* spawns the level's score pickup effect for this object */
void func_L16_0041F150(char *arg0) {
    char buf[0x10];
    char *st = *(char **)(arg0 + 0x68);
    char *obj;
    s32 handle;
    s32 a;

    func_002E74B8(buf, arg0 + 0x10, st);
    handle = func_00313EE0(3, 7) & 0xFF;
    a = func_00313EA0(0xFF) & 0xFF;
    obj = (char *)func_003320E0(buf, st, 0x60006080, handle, 2.5f, a, 0);
    if (obj == 0)
        return;
    obj[9] = (u8)(func_002E8648(4.0f) - 16);
}

INCLUDE_ASM("asm/levels", func_L16_0041F1F0);

INCLUDE_ASM("asm/levels", func_L16_0041F360);

INCLUDE_ASM("asm/levels", func_L16_00424B48);

INCLUDE_ASM("asm/levels", func_L16_00435CE8);
