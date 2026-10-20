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
} T_001B99C0;

extern T_001B99C0 D_L16_001B99C0;

extern void func_002A7470(void *, s32, s32, s32);

INCLUDE_ASM("asm/levels", func_L16_002C6378);

/* func_L16_002D2D10: six spellings give the same 72-byte object (retail is 88).
   Deposited in nonmatching/levels/func_L16_002D2D10.c. */
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

extern void func_002A7750(char *, s32, s32);

extern s32 func_002A9640(char *, s32, s32);

extern void func_L16_003F55A0(char *);

extern void func_L16_003F5708(char *);

extern void func_002E5CF8(s32, s32, char *);

extern void func_0029F338(char *);

/* Tears the resident down (state 0) or starts it once the readiness test passes
   (state 1), chosen by the record's own state byte. */
void func_L16_003F54F8(char *a0) {
    char *s0 = a0;

    switch (*(u8 *)(s0 + 0x20)) {
    case 0:
        *(u16 *)(s0 + 0x32) = 0x200;
        *(u16 *)(s0 + 0x34) |= 0x4000;
        func_002A7750(s0, 1, -1);
        break;
    case 1:
        if (func_002A9640(s0, 0x80000, 0) != 0) {
            func_L16_003F55A0(s0);
            func_L16_003F5708(s0);
            func_002E5CF8(0, 0, s0);
            func_0029F338(s0);
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L16_003F55A0);

INCLUDE_ASM("asm/levels", func_L16_003F5708);
