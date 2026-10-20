#include "common.h"

/* 989snd's vsync callback, permcb.cpp in the NTSC split.
   rac1-decomp src/core/permcb.c: func_0012F308. The three counters sit at
   0x001A7200/0x001A7208/0x001A7210; retail reaches the last one $gp-relative
   because its store lands in the jr delay slot (0x001A7210 = $gp - 0x7DE0).
   0x10000800 is T0_COUNT. */
extern long D_001A7200 NOT_SDA;
extern long D_001A7208 NOT_SDA;
extern long D_001A7210 MACRO_ADDR;

int func_001339B0(void) {
    D_001A7208++;
    D_001A7210 = D_001A7200 + (unsigned int)*(volatile int *)0x10000800;
    return 0;
}
