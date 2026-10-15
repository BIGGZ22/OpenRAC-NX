#include "common.h"

extern void func_002AB8C0(u8 *a0, void *a1);

extern void func_00282978(void *a0);

extern void func_002A7470(u8 *a0, u32 a1, u32 a2, u32 a3);

extern void func_002AEC48(u8 *a0, void *a1, f32 f12, f32 f13, s32 a2);

extern void func_002B0990(u8 *a0, void *a1, f32 f12, s32 a2, s32 a3);

extern void *D_L11_001FC420[] __attribute__((sda));

extern u8 *D_L11_001B359C;

extern void func_0029FCC0(u32 a0, s32 a1, f32 *a2);

extern f32 func_00283180(f32 a0);

extern f32 func_00283198(f32 a0);

extern void func_00282CC0(f32 *a0, f32 *a1, f32 *a2);

extern void func_L11_00431EE8(u8 *a0, f32 *a1);

extern void func_L11_004320F8(u8 *a0, f32 *a1);



INCLUDE_ASM("asm/levels", func_L11_00425F10);

INCLUDE_ASM("asm/levels", func_L11_004295A0);

INCLUDE_ASM("asm/levels", func_L11_0042A158);

/* Same preparation as func_L11_00431D50 with the class setup on 0x260 and a two-stage
   animation seeded on 0x2A0. */
void func_L11_0042A420(u8 *arg0) {
    u8 *s0 = *(u8 **)(arg0 + 0x68);

    func_002AB8C0(arg0, s0 + 0x50);
    func_00282978(s0 + 0x260);
    func_00282978(s0 + 0x26);
    if (*(u16 *)(arg0 + 0x96) <= 0xFFFE)
        (*(u16 *)(arg0 + 0x96))++;
    func_002AEC48(arg0, s0 + 0x2A0, 0.03f, 0.3f, 4);
    func_002B0990(arg0, s0 + 0x2A0, 3.5f, 4, 1);
}

INCLUDE_ASM("asm/levels", func_L11_0042A4C8);

INCLUDE_ASM("asm/levels", func_L11_0042A5B0);

/* Applies the class setup to this object and its owner, or to this object alone when they
   are the same. */
void func_L11_0042EF90(u8 *arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4) {
    u8 *s2 = *(u8 **)(arg0 + 0x68);

    if (*(u32 *)(s2 + 0x27C) != 0) {
        func_002A7470(arg0, arg1, arg3, arg4);
        func_002A7470(*(u8 **)(s2 + 0x27C), arg2, arg3, arg4);
    } else if (arg0[0x43] != arg1) {
        func_002A7470(arg0, arg1, arg3, arg4);
    }
}

INCLUDE_ASM("asm/levels", func_L11_0042F018);

INCLUDE_ASM("asm/levels", func_L11_00431860);

/* Runs the class setup on the sub-record, unmarks two state halfwords and steps the timer. */
void func_L11_00431D50(u8 *arg0) {
    u8 *s0 = *(u8 **)(arg0 + 0x68);

    func_002AB8C0(arg0, s0 + 0x50);
    func_00282978(s0 + 0x270);
    func_00282978(s0 + 0x272);
    if (*(u16 *)(arg0 + 0x96) <= 0xFFFE)
        (*(u16 *)(arg0 + 0x96))++;
}

/* func_L11_00431DB0 lives in L11_00425F10-g8.c (it reaches D_L11_001B359C
   through $gp). */

/* Builds the object's follow vector from the renderer's position, offsets its height and
   hands it to the two transform helpers. */
void func_L11_00431E30(u8 *arg0) {
    u8 *s0 = arg0;
    u8 *s1 = *(u8 **)(s0 + 0x68);
    u8 *p = *(u8 **)(s1 + 0x27C);
    u8 *q;
    f32 v[3];
    f32 w[3];

    if (p == 0)
        return;
    q = *(u8 **)(p + 0x68);
    func_0029FCC0(*(u32 *)(q + 0x278), 0, v);
    w[0] = func_00283180(*(f32 *)(s0 + 0xF8)) * 0.1f;
    w[1] = func_00283198(*(f32 *)(s0 + 0xF8)) * 0.1f;
    w[2] = 0.0f;
    func_00282CC0(v, v, w);
    v[2] = v[2] - 0.05f;
    func_L11_00431EE8(s0, v);
    func_L11_004320F8(s0, v);
}

INCLUDE_ASM("asm/levels", func_L11_00431EE8);

INCLUDE_ASM("asm/levels", func_L11_004320F8);
