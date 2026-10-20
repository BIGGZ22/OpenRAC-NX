#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern s32 func_2F36F0(void *);
extern void func_3330A8(void *);
extern void func_383120(void *, void *);
extern f32 func_2F4A88(f32);
extern f32 func_2F4088(f32);
extern void func_2F3A80(void *, void *, void *);
extern void func_2F3AB0(void *, void *, void *);
extern void func_2F3AF0(void *, s32, f32);
extern s32 func_2F4BE0(u32, u8, u8, f32);
extern f32 func_2F4BD0(s32, s32);
extern void *func_32F2B0(void *);
extern void func_32F7A8(void *, void *, void *, void *, s32);
extern void *func_332F58(s32);

extern u8 *D_001FF814;

INCLUDE_ASM("asm/levels", func_L22_002D0C98);

/* VU0 macro-op helper (lqc2/sdc2 + a COP2 op): hand-written assembly. */
INCLUDE_ASM("asm/levels", func_L22_002F3B38);

/* Early-outs when the third float argument is positive; otherwise bends the first angle
   through one helper, feeds the second angle and the angle at the fourth pointer through
   the two placement calls, scales it by the first float argument and stores it back. */
void func_L22_00328220(u8 *a0, u8 *a1, u8 *a2, u8 *a3, f32 f12, f32 f13, f32 f14)
{
    u8 *s0 = a0;
    u8 *s1 = a1;
    u8 *s2 = a3;
    u8 *s3 = a2;
    TI128 buf;
    f32 keep;
    f32 q;

    if (f14 > 0.0f)
        return;
    keep = f12;
    *(f32 *)(s1 + 0) = func_2F4A88(*(f32 *)(s1 + 0));
    func_2F3AF0(&buf, s2, *(f32 *)(s3 + 0));
    s0 += 0x10;
    func_2F3AB0(s0, s0, &buf);
    q = func_2F4088(*(f32 *)(s1 + 0)) * keep;
    *(f32 *)(s3 + 0) = q;
    func_2F3AF0(&buf, s2, q);
    func_2F3A80(s0, s0, &buf);
}

INCLUDE_ASM("asm/levels", func_L22_0032F228);

INCLUDE_ASM("asm/levels", func_L22_0032F2C0);

INCLUDE_ASM("asm/levels", func_L22_003300E8);

INCLUDE_ASM("asm/levels", func_L22_00334F78);

void func_L22_00335078(u8 *a0)
{
    u8 *s0 = a0;
    u8 *s1 = a0 + 0x20;

    if (func_2F36F0(s1) != 0) {
        func_3330A8(s0);
        return;
    }
    {
        u8 *p = s0 + 0x10;

        func_383120(p, p);
    }
    *(f32 *)(s0 + 0xC) = *(f32 *)(s0 + 0xC) + *(f32 *)(s1 + 4);
    if (*(s32 *)(s1) < 6) {
        *(u32 *)(s0 + 4) = (*(u32 *)(s0 + 4) & 0xFFFFFF)
            | ((u32)(*(s32 *)(s1) * 127 / 6) << 24);
    }
}

INCLUDE_ASM("asm/levels", func_L22_003398B8);

INCLUDE_ASM("asm/levels", func_L22_0039CDC0);

INCLUDE_ASM("asm/levels", func_L22_0039D198);
