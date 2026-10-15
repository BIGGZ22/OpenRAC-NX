#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad[0x274];
    f32 f274;
    f32 f278;
    f32 f27C;
    s32 i280;
} T_001BFF99C0;

extern T_001BFF99C0 D_001BFF99C0;

extern void func_002A7470(void *, s32, s32, s32);

INCLUDE_ASM("asm/levels", func_L16_002C6378);

INCLUDE_ASM("asm/levels", func_L16_002D2D10);

INCLUDE_ASM("asm/levels", func_L16_002F60B8);

INCLUDE_ASM("asm/levels", func_L16_00317C90);

/* runs the readiness helper once when the handle is in state 1 with flag 2 set */
void func_L16_003AA0C8(u8 *arg0) {
    if (arg0[0x43] == 1 && (arg0[0x60] & 2) != 0) {
        func_002A7470(arg0, 0, 0, 1);
    }
}

INCLUDE_ASM("asm/levels", func_L16_003AA108);

INCLUDE_ASM("asm/levels", func_L16_003AA3F0);

INCLUDE_ASM("asm/levels", func_L16_003D6D00);

INCLUDE_ASM("asm/levels", func_L16_003E6E08);

INCLUDE_ASM("asm/levels", func_L16_003E70E0);

INCLUDE_ASM("asm/levels", func_L16_003E7248);

INCLUDE_ASM("asm/levels", func_L16_003F4FA8);

INCLUDE_ASM("asm/levels", func_L16_003F54F8);

INCLUDE_ASM("asm/levels", func_L16_003F55A0);

INCLUDE_ASM("asm/levels", func_L16_003F5708);
