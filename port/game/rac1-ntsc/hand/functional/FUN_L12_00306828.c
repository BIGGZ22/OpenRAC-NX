/* Carried from the PAL decompilation's nonmatching/functional/func_L12_00307C08.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* functional: high. Written from the retail instructions (420 bytes); not a match. Retail traps on a zero tick count in the modulo; here it takes 0. */
/* Level 12. Pulses the colour of a moby and its 17 children (data + 0x20 + i * 0x30 holds each
 * child's index; skipped when of class 0x4FE): a sine over ticks(170) frames sets the colour word
 * at +0x90 (red and green 180 + 70 sin, blue 70 + 10 sin, alpha bit 31) and flag 0x10 at +0x34. */
extern char *D_L12_00160058_307C08 __asm__("D_L12_0015FFD8");
extern int D_L12_0015F6B0_307C08 __asm__("D_L12_0015F5CC");
extern int func_001F9850_307C08(int) __asm__("func_001F9688");
extern float func_001FA888_307C08(int) __asm__("func_001FA6C0");
extern float func_001F9FA8_307C08(float) __asm__("func_001F9DE0");
extern int func_001FA898_307C08(float) __asm__("func_001FA6D0");

void FUN_L12_00306828(char *moby) {
    char *d = *(char **)(moby + 0x78);
    int i;

    for (i = 0; i < 0x12; i++) {
        char *m;
        int n, r, a, b, c;
        float fn, x, s;

        if (i == 0x11) {
            m = moby;
        } else {
            m = D_L12_00160058_307C08 + (*(int *)(d + i * 0x30 + 0x20) << 8);
        }
        if (*(short *)(m + 0xA6) == 0x4FE) {
            continue;
        }
        n = func_001F9850_307C08(0xAA);
        fn = func_001FA888_307C08(n);
        r = n != 0 ? D_L12_0015F6B0_307C08 % n : 0;
        x = (float)r / fn;
        x = x + x;
        x = x * 3.14159274f;
        s = func_001F9FA8_307C08(x + -3.14159274f);
        a = func_001FA898_307C08(s * 70.0f);
        b = func_001FA898_307C08(s * 70.0f);
        c = func_001FA898_307C08(s * 10.0f);
        b += 0xB4;
        if (b >= 0x100) {
            b = 0xFF;
        }
        a += 0xB4;
        if (a >= 0x100) {
            a = 0xFF;
        }
        *(unsigned int *)(m + 0x90) = 0x80000000u | ((unsigned int)(c + 0x46) << 16) | ((unsigned int)b << 8) | (unsigned int)a;
        *(unsigned short *)(m + 0x34) |= 0x10;
    }
}
