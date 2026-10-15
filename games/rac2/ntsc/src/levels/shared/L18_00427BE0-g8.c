#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned short u16;

typedef signed short s16;

typedef signed int s32;

typedef float f32;

/* Small-data controls this level reads through $gp in ordinary flow. */
extern s32 D_L18_001B1254 __attribute__((sda));

extern s32 D_L18_001B1258 __attribute__((sda));

extern s32 D_L18_001B125C __attribute__((sda));

extern f32 D_L18_001B12FC __attribute__((sda));

extern f32 D_L18_001B12C0 __attribute__((sda));

extern f32 D_L18_001B1C4C __attribute__((sda));

extern f32 D_L18_001B1C48 __attribute__((sda));

extern f32 D_L18_001B1C44 __attribute__((sda));

extern f32 D_L18_001B1C40 __attribute__((sda));

extern s32 func_002A7820(s32, s32);

extern void func_00282978(void *);

extern void func_00282948(s32);

extern void func_002B0990(void *, void *, s32, s32, f32);

extern void func_0029FCC0(void *, s32, void *);

extern s32 func_00337AD8(s32, s32, f32);

extern void func_L00_00325AE8(void *, s32, f32, f32);

/* Renews the state's 0x39A banner: resamples the 0x390 countdown, refreshes the two
   clearings and bumps the object's own 0x96 counter before the shared follow-up. */
/* Asks the shared vertex probe for the object's 0x78 mask and places the resulting
   height with the state's own 0x158 rise. */
void func_L18_0043E938(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    if (D_L18_001B1C4C < *(f32 *)(state + 0x15C)) {
        s32 probe = func_00337AD8(*(s32 *)(obj + 0x78) & 0xFFFFFF, *(s32 *)(obj + 0x78),
                                  *(f32 *)(state + 0x15C));
        func_L00_00325AE8(obj + 0x10, probe,
                          D_L18_001B1C44 + D_L18_001B1C48 * *(f32 *)(state + 0x158),
                          D_L18_001B1C40);
    }
}

/* hands the two sampled buffers and the measured flag pair to the shared placement. */
extern s32 func_L01_003120A8(void *, void *, void *, s32, f32, f32, s32, s32, s32);
