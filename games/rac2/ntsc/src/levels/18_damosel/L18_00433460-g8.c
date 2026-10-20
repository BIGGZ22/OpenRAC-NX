#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;
typedef int TI128 __attribute__((mode(TI)));

extern u16 D_L18_001B18FC __attribute__((sda));

extern float D_L18_001B191C __attribute__((sda));

extern void *func_0029EFD0(short a);

extern float func_00283248(float a, float b);

extern void func_002A0F48(void *a);

extern int func_002A7750(void *, int, int);

extern void *func_002A9640(void *, s32, s32);

extern void func_0029F338(void *);

extern void func_L00_0032A258(void *, void *, void *, void *, s32, s32, s32, s32,
                              s32, s32, s32);

extern void func_0032C700(void *, s32, s32, s32, s32, s32, s32, s32, float, float,
                          s32, s32, s32, s32);

extern float D_L18_001B1D30 __attribute__((sda));

extern float D_L18_001B1D38 __attribute__((sda));

extern float D_L18_001B1D40 __attribute__((sda));

void func_L18_004412F0(u8 *s0) {
    s32 state = s0[0x20];
    u8 *s1 = *(u8 **)(s0 + 0x68);
    s32 one = 1;
    switch (state) {
    case 0:
        func_002A7750(s0, 1, -1);
        break;
    case 1:
        if (func_002A9640(s0, 0x04010000, 0) != 0 && *(s32 *)s1 == 0) {
            func_002A7750(s0, 2, -1);
        }
        s0[0xA8] = 0xFF;
        break;
    case 2: {
        s32 v = 0xFFFF;
        func_L00_0032A258(s0, &D_L18_001B1D40, &D_L18_001B1D38,
                          &D_L18_001B1D30, 0xFFFF, 0, -1, 0, v, 0xFF, 0);
        func_0032C700(s0 + 0x10, 0, 5, 5, 5, 5, 5, 0,
                      *(float *)(s0 + 0x2C) * 10.0f, 20.0f, one, 0, one, 0);
        func_0029F338(s0);
        break;
    }
    }
}

INCLUDE_ASM("asm/levels", func_L18_00437AA0);
