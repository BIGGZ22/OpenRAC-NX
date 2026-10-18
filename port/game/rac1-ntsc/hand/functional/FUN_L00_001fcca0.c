/* functional: high. Checked against the US assembly (0x1FCCA0..0x1FCF88): fields +0/4/6/8/0xA/0xC/0xE, the fade ramps (0.0625 and 0.125), the ' - ' join in mode 1, the 0x1A row advance otherwise, and the calls to FUN_001fdd10 (0x203AD8 in this level) and 0x1FCB28 with f12 = f21 and f13 = f20 match PAL's C. */
/* Carried from the PAL decompilation's nonmatching/shared/func_L00_001FCC50.c (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
void func_L00_001FCB28(int x, int y, int mode, unsigned char *p, float scale, float alpha);
extern unsigned char *func_001FDD10(int);



/* Draws a line-wrapped text entry with fade-in/out at its scroll edges (newline codes become " - " or start a new row). */
void FUN_L00_001fcca0(void *line, int t) {
    char *r = line;
    int x8 = *(short *)(r + 8);
    int end = x8 + *(short *)(r + 0xA);
    float f21, f20;
    unsigned char *p;
    char buf[256];
    char *d;
    char c;
    int y;

    if (t < x8) return;
    if (end < t) return;
    f21 = 1.0f;
    if (t < x8 + 0x10) f21 = (float)(t - x8) * 0.0625f;
    if (end - 8 < t) f21 = (float)(end - t) * 0.125f;
    f20 = 1.0f;
    if (t < x8 + 0x20) {
        f20 = (float)(t - 0x10 - x8) * 0.0625f;
        if (f20 < 0.0f) f20 = 0.0f;
    }
    if (end - 0x10 < t) {
        f20 = (float)(end - (t + 8)) * 0.125f;
        if (f20 < 0.0f) f20 = 0.0f;
    }
    p = func_001FDD10(*(int *)r);
    d = buf;
    if (*(short *)(r + 0xE) == 1) {
        c = *p;
        while (c != 0) {
            while (c != 0 && c != 1) {
                *d++ = c;
                p++;
                c = *p;
            }
            if (c == 1) {
                *d++ = 0x20;
                *d++ = 0x2D;
                *d++ = 0x20;
                while (*p == 1) p++;
                c = *p;
                if (c == 0) {
                    d -= 3;
                    break;
                }
            }
        }
        *d = 0;
        func_L00_001FCB28(*(short *)(r + 4), *(short *)(r + 6), *(short *)(r + 0xC), buf, f21, f20);
    } else {
        y = *(short *)(r + 6);
        c = *p;
        while (c != 0) {
            if (c == 1) {
                *d = 0;
                func_L00_001FCB28(*(short *)(r + 4), y, *(short *)(r + 0xC), buf, f21, f20);
                y += 0x1A;
                d = buf;
                while (*p == 1) p++;
                c = *p;
            } else {
                *d++ = c;
                p++;
                c = *p;
            }
        }
        if (d > buf) {
            *d = 0;
            func_L00_001FCB28(*(short *)(r + 4), y, *(short *)(r + 0xC), buf, f21, f20);
        }
    }
}
