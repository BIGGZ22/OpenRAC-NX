#include "common.h"

/* Small-data unit: every body here reaches a global through $gp, so the file is built
   with -G8 (the -g8.c name gives it that profile). */

extern float D_L30_001AEE90 __attribute__((sda));
extern float D_L30_001AEE94 __attribute__((sda));
extern char D_L30_001AEE98[];

extern void func_0026FEB8(const char *, ...);
extern void func_001163A0(unsigned int);
extern int func_002A77E0(int);
extern void func_002A7750(u8 *, int, int);
extern void func_002AEE70(u8 *, u8 *, u8 *, float, float, float);
extern float func_00283B98(float, float);

/* Steps one record: with the state byte clear it copies the record's own angle into the
   state block, and once the state index reaches thirteen it redraws the record with a fresh
   random duration, a timer sample and the state block's new bit mask. With the state byte
   set it folds the shared angle words into the record and the state block instead, and
   counts the frame in the record's halfword at 0x96. */
void func_L30_00386300(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u16 v;

    if (*(u8 *)(object + 0x20) == 0) {
        float f = *(float *)(object + 0x18);
        unsigned int w = *(unsigned int *)state;

        *(float *)(state + 0xC) = f;
        if (w >= 13) {
            short a1 = *(short *)(object + 0xAA);
            short a2 = *(short *)(object + 0xB2);
            unsigned int ticks = *(volatile unsigned int *)0x10000800;

            func_0026FEB8(D_L30_001AEE98, a1, a2);
            func_001163A0(ticks);
            *(int *)state = func_002A77E0(13);
        }
        *(u16 *)(object + 0x7C) = 1 << *(int *)state;
        func_002A7750(object, 1, -1);
    } else {
        float f0 = D_L30_001AEE94;

        func_002AEE70(object, state + 4, state + 8, D_L30_001AEE90,
                      f0 * 0.000290888245f, *(float *)(state + 0xC));
        *(float *)(object + 0xF8) =
            func_00283B98(*(float *)(object + 0xF8), 0.01745329424738884f);
        *(u16 *)(object + 0x96) = 0;
    }
    v = *(u16 *)(object + 0x96);
    *(u16 *)(object + 0x96) = v + 1;
}
