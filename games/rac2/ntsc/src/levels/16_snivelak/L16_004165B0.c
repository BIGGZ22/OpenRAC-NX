#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

INCLUDE_ASM("asm/levels", func_L16_004165B0);

INCLUDE_ASM("asm/levels", func_L16_00416D08);

INCLUDE_ASM("asm/levels", func_L16_00416EB8);

INCLUDE_ASM("asm/levels", func_L16_00416EC8);

INCLUDE_ASM("asm/levels", func_L16_00416FB0);

INCLUDE_ASM("asm/levels", func_L16_00417210);

INCLUDE_ASM("asm/levels", func_L16_00417788);

INCLUDE_ASM("asm/levels", func_L16_00417798);

INCLUDE_ASM("asm/levels", func_L16_00417990);

INCLUDE_ASM("asm/levels", func_L16_00417DB0);

INCLUDE_ASM("asm/levels", func_L16_004182A8);

INCLUDE_ASM("asm/levels", func_L16_004185C0);

INCLUDE_ASM("asm/levels", func_L16_004189A0);

INCLUDE_ASM("asm/levels", func_L16_00418D28);

INCLUDE_ASM("asm/levels", func_L16_00418E38);

INCLUDE_ASM("asm/levels", func_L16_00419140);

INCLUDE_ASM("asm/levels", func_L16_004192A8);

INCLUDE_ASM("asm/levels", func_L16_0041C6A0);

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
