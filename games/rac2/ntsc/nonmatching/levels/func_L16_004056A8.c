#include "include_asm.h"

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern s32 func_313EE0(s32, s32);

extern f32 func_314000(void);

extern void func_405818(void *, s32, s32, s32, s32, void *, f32, f32, f32);

extern void func_414038(void *, s32, s32, s32, s32, void *, f32, f32, f32);

extern s32 D_L16_001B1460 __attribute__((sda));

extern s32 D_L16_001B1464 __attribute__((sda));

extern s32 D_L16_001B1498 __attribute__((sda));

extern s32 D_L16_001B149C __attribute__((sda));

extern f32 D_L16_001B14A0 __attribute__((sda));

extern f32 D_L16_001B14A4 __attribute__((sda));

extern s32 D_L16_001B1908 __attribute__((sda));

extern s32 D_L16_001B190C __attribute__((sda));

extern s32 D_L16_001B1940 __attribute__((sda));

extern s32 D_L16_001B1944 __attribute__((sda));

extern f32 D_L16_001B1948 __attribute__((sda));

extern f32 D_L16_001B194C __attribute__((sda));

/* Emits the measured two-element effect table around the first pair of handles. */

void func_L16_004056A8(u8 *arg)
{
    s32 h;
    u8 *p;
    u8 *q;
    f32 v;
    f32 one = 1.0f;

    p = arg;
    q = p + 0x10;
    v = D_L16_001B14A0 * *(f32 *)(p + 0x2C);
    h = func_313EE0(0x5F, 0x6F);
    func_405818(q, D_L16_001B1460, h, D_L16_001B1498, 2, p, v,
                func_314000(), one);
    v = D_L16_001B14A0 * *(f32 *)(p + 0x2C) * D_L16_001B14A4;
    h = func_313EE0(0x6F, 0x7F);
    func_405818(q, D_L16_001B1464, h, D_L16_001B149C, 2, p, v,
                func_314000(), one);
}
