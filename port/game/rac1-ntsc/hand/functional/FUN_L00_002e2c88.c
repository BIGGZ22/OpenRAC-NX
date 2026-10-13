/* Carried from the PAL decompilation's nonmatching/functional/func_L00_002E4138.c (games/rac1/pal) through the version
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
/* functional: high. Same reads, writes, calls and arguments in the same order as retail, read side by side with the asm; only compiler choices differ (registers, which frame pointers retail spills to 0x1F0-0x218 and reloads, the float table read through a5*4 recomputed), and the four loop-copy sources are the frame vectors themselves rather than pointers re-read from spill slots the near miss never wrote. */
/* FUN_L00_002e2c88 -- src/overlays/shared/vendor_002E1660.c (functional C for the port, not a match)
 * Effect beam builder for slot a5: builds two 0x90-byte draw frames (fr+0x00 and fr+0x90: four quads,
 * four colours from a3/a4 alphas, a UV block from D_L00_00161C78[a5], two 64-bit packet words from
 * func_001F44B8(a0)/(a1)), a basis from the D_L00_001E6D30/D_L00_001E6DE0 tables and the point list a2,
 * then for i = 1..18 rotates the next segment (a2[i], a2[i+1]) into place, draws both frames with
 * func_L00_001FD228 (the first segment with white/red head colours), and narrows the width
 * from 0.6 (0.3 when the moby D_L00_00161C88[a5] has class 0x1C) toward 0.05 by func_L00_00257E20.
 * equiv: the near miss read the loop-copy sources from frame slots 0x208/0x20C/0x214/0x218 that it never
 * stored (retail spills fr+0x20, +0x30, +0xB0, +0xC0 there); fixed by using those vectors directly.
 */
typedef int u128_2E4138_c2e2c88 __attribute__((mode(TI)));

extern long func_001F4868_2E4138(int) __asm__("func_001F44B8");
extern void func_001F9BF0_2E4138(void *, void *, void *) __asm__("func_001F9A28");
extern void func_001F9C30_2E4138(void *, void *, float) __asm__("func_001F9A68");
extern void func_001F9CA0_2E4138(void *, void *, void *) __asm__("func_001F9AD8");
extern void func_L00_001FF4B0_2E4138(void *, void *, float) __asm__("func_L00_001FF500");
extern void func_001F9EE8_2E4138(void *, void *, void *) __asm__("func_001F9D20");
extern void func_L00_001FD1D8_2E4138(void *, int, int) __asm__("func_L00_001FD228");
extern float func_001FA888_2E4138(int) __asm__("func_001FA6C0");
extern float func_L00_00258E58_2E4138(float, float, float, float, float) __asm__("func_L00_00257E20");
extern float D_L00_00161D48_2E4138[] __asm__("D_L00_00161C78") MACRO_ADDR;
extern int D_L00_00161D58_2E4138[] __asm__("D_L00_00161C88") MACRO_ADDR;
extern char D_L00_001E7080_2E4138[] __asm__("D_L00_001E6D30") NOT_SDA;
extern char D_L00_001E7130_2E4138[] __asm__("D_L00_001E6DE0") NOT_SDA;
extern float D_L00_00166EC0_2E4138[] __asm__("D_L00_00166DC0");
extern char D_0013E633_2E4138[] __asm__("D_0013E533");

#define Q_2E4138(p) (*(u128_2E4138_c2e2c88 *)(p))

void FUN_L00_002e2c88(int a0, int a1, char *a2, int a3, int a4, int a5)
{
    char fr[0x1F0] __attribute__((aligned(16)));
    long r0, r1, c88, c80, z70;
    float one, zero, w, mone, uv, u0, u1, t, s;
    int ca, cb, sa, sb, i;
    char *p0, *p1;

    r0 = func_001F4868_2E4138(a0);
    one = 1.0f;
    ca = (a3 << 24) | 0x7F7F7F;
    c88 = ((long)0x8000 << 24) | 0x48;
    c80 = ((long)0xFF90 << 32) | 0x260;
    *(long *)(fr + 0x88) = c88;
    *(long *)(fr + 0x80) = c80;
    *(long *)(fr + 0x78) = r0;
    *(int *)(fr + 0x40) = ca;
    *(long *)(fr + 0x70) = 0;
    *(int *)(fr + 0x4C) = ca;
    *(int *)(fr + 0x48) = ca;
    *(int *)(fr + 0x44) = ca;
    r1 = func_001F4868_2E4138(a1);

    w = 0.6f;
    *(int *)(fr + 0xE4) = 0;
    cb = (a4 << 24) | 0x7F4040;
    uv = D_L00_00161D48_2E4138[a5];
    zero = *(float *)(fr + 0xE4);
    u1 = uv + one;
    u0 = uv + zero;
    c88 = *(long *)(fr + 0x88);
    c80 = *(long *)(fr + 0x80);
    z70 = *(long *)(fr + 0x70);
    *(long *)(fr + 0x108) = r1;
    *(int *)(fr + 0xD0) = cb;
    *(long *)(fr + 0x118) = c88;
    *(long *)(fr + 0x110) = c80;
    *(long *)(fr + 0x100) = z70;
    *(float *)(fr + 0x58) = u0;
    *(float *)(fr + 0x68) = u1;
    i = D_L00_00161D58_2E4138[a5];
    *(int *)(fr + 0xDC) = cb;
    *(int *)(fr + 0xD8) = cb;
    *(int *)(fr + 0xD4) = cb;
    *(float *)(fr + 0xE0) = one;
    *(float *)(fr + 0xE8) = one;
    *(float *)(fr + 0xEC) = one;
    *(int *)(fr + 0xF0) = 0;
    *(int *)(fr + 0xF4) = 0;
    *(int *)(fr + 0xF8) = 0;
    *(float *)(fr + 0xFC) = one;
    *(float *)(fr + 0x50) = u0;
    *(float *)(fr + 0x54) = zero;
    *(float *)(fr + 0x5C) = one;
    *(float *)(fr + 0x60) = u1;
    *(float *)(fr + 0x64) = zero;
    *(float *)(fr + 0x6C) = one;
    if (*(short *)(i + 0xA6) == 0x1C) {
        w = 0.3f;
    }

    Q_2E4138(fr + 0x120) = 0;
    Q_2E4138(fr + 0x130) = 0;
    *(float *)(fr + 0x138) = w + w;
    *(float *)(fr + 0x12C) = one;
    *(float *)(fr + 0x128) = w;
    *(float *)(fr + 0x13C) = one;
    Q_2E4138(fr + 0x140) = Q_2E4138(D_L00_001E7080_2E4138 + (a5 << 6));
    p0 = D_L00_001E7080_2E4138 + 0x20 + (a5 << 6);
    func_001F9BF0_2E4138(fr + 0x150, D_L00_00166EC0_2E4138, p0);
    func_001F9C30_2E4138(fr + 0x150, fr + 0x150, one);
    func_001F9CA0_2E4138(fr + 0x160, fr + 0x150, fr + 0x140);
    func_L00_001FF4B0_2E4138(fr + 0x160, fr + 0x160, one);
    func_001F9CA0_2E4138(fr + 0x140, fr + 0x160, fr + 0x150);
    Q_2E4138(fr + 0x170) = Q_2E4138(p0);
    Q_2E4138(fr + 0x1B0) = Q_2E4138(p0);
    mone = -1.0f;
    func_001F9BF0_2E4138(fr + 0x1C0, D_L00_001E7130_2E4138 + (a5 << 4), fr + 0x1B0);
    func_L00_001FF4B0_2E4138(fr + 0x180, fr + 0x1C0, one);
    func_001F9CA0_2E4138(fr + 0x190, fr + 0x180, D_0013E633_2E4138 + 0x10AD);   /* 0x13F5E0 */
    func_L00_001FF4B0_2E4138(fr + 0x190, fr + 0x190, mone);
    func_001F9CA0_2E4138(fr + 0x1A0, fr + 0x190, fr + 0x180);
    Q_2E4138(fr + 0x00) = Q_2E4138(fr + 0x120);
    func_001F9C30_2E4138(fr + 0x10, fr + 0x00, mone);
    Q_2E4138(fr + 0x90) = Q_2E4138(fr + 0x130);
    func_001F9C30_2E4138(fr + 0xA0, fr + 0x90, mone);

    func_001F9EE8_2E4138(fr + 0x1D0, a2, fr + 0x180);
    func_001F9EE8_2E4138(fr + 0x1E0, a2 + 0x10, fr + 0x180);
    func_001F9BF0_2E4138(fr + 0x140, fr + 0x1E0, fr + 0x1D0);
    func_L00_001FF4B0_2E4138(fr + 0x140, fr + 0x140, one);
    func_001F9BF0_2E4138(fr + 0x150, D_L00_00166EC0_2E4138, fr + 0x1D0);
    func_001F9CA0_2E4138(fr + 0x160, fr + 0x150, fr + 0x140);
    func_L00_001FF4B0_2E4138(fr + 0x160, fr + 0x160, one);
    func_001F9CA0_2E4138(fr + 0x150, fr + 0x140, fr + 0x160);
    Q_2E4138(fr + 0x170) = Q_2E4138(fr + 0x1D0);
    func_001F9EE8_2E4138(fr + 0x00, fr + 0x00, fr + 0x140);
    func_001F9EE8_2E4138(fr + 0x10, fr + 0x10, fr + 0x140);
    func_001F9EE8_2E4138(fr + 0x90, fr + 0x90, fr + 0x140);
    func_001F9EE8_2E4138(fr + 0xA0, fr + 0xA0, fr + 0x140);

    p0 = a2 + 0x10;
    p1 = a2 + 0x20;
    i = 1;
    do {
        Q_2E4138(fr + 0x20) = Q_2E4138(fr + 0x120);
        func_001F9C30_2E4138(fr + 0x30, fr + 0x20, mone);
        Q_2E4138(fr + 0xB0) = Q_2E4138(fr + 0x130);
        func_001F9C30_2E4138(fr + 0xC0, fr + 0xB0, mone);
        func_001F9EE8_2E4138(fr + 0x1D0, p0, fr + 0x180);
        func_001F9EE8_2E4138(fr + 0x1E0, p1, fr + 0x180);
        func_001F9BF0_2E4138(fr + 0x140, fr + 0x1E0, fr + 0x1D0);
        func_L00_001FF4B0_2E4138(fr + 0x140, fr + 0x140, one);
        func_001F9BF0_2E4138(fr + 0x150, D_L00_00166EC0_2E4138, fr + 0x1D0);
        func_001F9CA0_2E4138(fr + 0x160, fr + 0x150, fr + 0x140);
        func_L00_001FF4B0_2E4138(fr + 0x160, fr + 0x160, one);
        func_001F9CA0_2E4138(fr + 0x150, fr + 0x140, fr + 0x160);
        Q_2E4138(fr + 0x170) = Q_2E4138(fr + 0x1D0);
        func_001F9EE8_2E4138(fr + 0x20, fr + 0x20, fr + 0x140);
        func_001F9EE8_2E4138(fr + 0x30, fr + 0x30, fr + 0x140);
        func_001F9EE8_2E4138(fr + 0xB0, fr + 0xB0, fr + 0x140);
        func_001F9EE8_2E4138(fr + 0xC0, fr + 0xC0, fr + 0x140);
        if (i == 1) {
            sa = *(int *)(fr + 0x40);
            sb = *(int *)(fr + 0xD0);
            *(int *)(fr + 0x40) = 0xFFFFFF;
            *(int *)(fr + 0xD0) = 0x7F4040;
            *(int *)(fr + 0x44) = 0xFFFFFF;
            *(int *)(fr + 0xD4) = 0x7F4040;
            func_L00_001FD1D8_2E4138(fr + 0x00, 0, 0);
            func_L00_001FD1D8_2E4138(fr + 0x90, 0, 0);
            *(int *)(fr + 0x40) = sa;
            *(int *)(fr + 0xD0) = sb;
            *(int *)(fr + 0x4C) = sa;
            *(int *)(fr + 0x48) = sa;
            *(int *)(fr + 0x44) = sa;
            *(int *)(fr + 0xDC) = sb;
            *(int *)(fr + 0xD8) = sb;
            *(int *)(fr + 0xD4) = sb;
        } else {
            func_L00_001FD1D8_2E4138(fr + 0x00, 0, 0);
            func_L00_001FD1D8_2E4138(fr + 0x90, 0, 0);
        }
        Q_2E4138(fr + 0x00) = Q_2E4138(fr + 0x20);
        Q_2E4138(fr + 0x10) = Q_2E4138(fr + 0x30);
        Q_2E4138(fr + 0x90) = Q_2E4138(fr + 0xB0);
        Q_2E4138(fr + 0xA0) = Q_2E4138(fr + 0xC0);
        t = func_001FA888_2E4138(i);
        p1 += 0x10;
        i++;
        s = func_001FA888_2E4138(0x14);
        p0 += 0x10;
        t = func_L00_00258E58_2E4138(-1.0f, zero, one, zero, t / s);
        t = w + (0.05f - w) * t;
        uv = D_L00_00161D48_2E4138[a5];
        *(float *)(fr + 0x54) = zero;
        *(float *)(fr + 0x5C) = one;
        *(float *)(fr + 0x64) = zero;
        *(float *)(fr + 0x6C) = one;
        u1 = uv + one;
        u0 = uv + zero;
        *(float *)(fr + 0x68) = u1;
        *(float *)(fr + 0x58) = u0;
        *(float *)(fr + 0x50) = u0;
        *(float *)(fr + 0x60) = u1;
        *(float *)(fr + 0x128) = t;
        *(float *)(fr + 0x138) = t + t;
    } while (i < 19);
}
