#include "common.h"

INCLUDE_ASM("asm/levels", func_L12_0041E3E8);

s32 func_L12_0041E8E8(u8 *a0) {
    return *(u8 *)(a0 + 0x20) == 3;
}

extern int D_001A79F0 __attribute__((sda));
extern char D_0019B4A8[];
extern char D_00189EA0[];
extern void func_00282CF0(f32 *, const f32 *, const f32 *);
extern void func_002AFEB8(char *, char *, char *, char *, f32);
extern void func_00282F20(f32 *, f32 *, f32);
extern void func_002B0A68(void *, s32, void *, f32, f32, f32, f32, s32);

void func_L12_0041E8F8(u8 *a0) {
    u8 *self = a0;
    f32 buf[4];
    u16 f;

    func_00282CF0(buf, (f32 *)(self + 0x10), (f32 *)D_00189EA0);
    func_002AFEB8((char *)self, (char *)buf, (char *)buf, 0, 0.0f);
    func_00282F20(buf, buf, 0.10000000894069672f);
    func_002B0A68(self, 31, buf, 6.0f, 12.0f, 6.0f, 12.0f, 255);

    f = *(u16 *)(self + 0xB2);
    if (f & 1) {
        u8 *p = (u8 *)(((s16)f >> 1) + (D_001A79F0 << 10)) + (int)D_0019B4A8;
        u8 c = *p;

        if (((c >> 4) & 7) != 7)
            *p = c + 0x10;
    } else {
        u8 *p = (u8 *)(((s16)f >> 1) + (D_001A79F0 << 10)) + (int)D_0019B4A8;
        u8 c = *p;

        if ((c & 7) != 7)
            *p = c + 1;
    }
    func_002E5CF8(1, 0, self);
}

INCLUDE_ASM("asm/levels", func_L12_0041EA18);

INCLUDE_ASM("asm/levels", func_L12_0041EDB8);

INCLUDE_ASM("asm/levels", func_L12_00420660);

INCLUDE_ASM("asm/levels", func_L12_00420838);

INCLUDE_ASM("asm/levels", func_L12_00420950);

INCLUDE_ASM("asm/levels", func_L12_00420B78);

INCLUDE_ASM("asm/levels", func_L12_00421110);

INCLUDE_ASM("asm/levels", func_L12_004212C0);

INCLUDE_ASM("asm/levels", func_L12_00424370);

INCLUDE_ASM("asm/levels", func_L12_00425070);

INCLUDE_ASM("asm/levels", func_L12_00425178);

extern s32 *D_L12_001FEAC0[];
extern char D_00189EA0[];
extern f32 func_00282C48(f32);
extern f32 func_00282E80(void *, void *);

void func_L12_00425348(u8 *a0) {
    u8 *state = *(u8 **)(a0 + 0x68);
    s32 i = *(s32 *)(state + 0x2B4) + 1;
    u32 *entry = D_L12_001FEAC0[*(s32 *)(state + 0x298)];

    while (i < *(s32 *)entry) {
        if (func_00282C48(*(f32 *)((u8 *)entry + (i * 16 + 0x18)) - *(f32 *)((u8 *)entry + 0x18)) < 2.0f) {
            if (func_00282E80(D_00189EA0, (u8 *)entry + (i * 16 + 0x10)) < 6.0f)
                *(s32 *)(state + 0x2B4) = i;
        }
        i++;
    }
}

extern int func_00282948(void *);
extern s32 func_002A7820(s32, s32);
extern s32 func_002E5CF8(s32, s32, void *);
extern s32 D_L12_001B20C0 __attribute__((sda));

/* Starts the sub-object's timer when it has finished, then tells the engine to
   advance to mode 1.  The sub-object word at 0x001B20C4 is an absolute literal:
   written as a symbol it takes an extra register, and the gp one at 0x30D0 needs
   its `sda` declaration (measured). */
void func_L12_00425448(u8 *a0) {
    u8 *s0 = *(u8 **)(a0 + 0x68);

    if (func_00282948(s0 + 0x2C0) != 0) {
        *(s32 *)(s0 + 0x2C0) = func_002A7820(D_L12_001B20C0, (s32)*(u8 **)0x001B20C4);
        func_002E5CF8(1, 0, a0);
    }
}

INCLUDE_ASM("asm/levels", func_L12_004254A8);

INCLUDE_ASM("asm/levels", func_L12_004285A8);

INCLUDE_ASM("asm/levels", func_L12_004293A0);

INCLUDE_ASM("asm/levels", func_L12_00429610);

INCLUDE_ASM("asm/levels", func_L12_00429868);
