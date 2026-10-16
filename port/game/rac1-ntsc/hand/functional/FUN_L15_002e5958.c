/* Carried from the PAL decompilation's nonmatching/functional/func_L15_002E6D48.c (games/rac1) through the version
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
/* What the PAL file this was written in declares before it. */
extern void func_0022DA68(int, int, void *);

extern void func_001F9A28(void *, void *, void *);
extern float func_001F9AF0(void *a);
extern void func_001FA2D8(void *, void *);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_0024F7C8(void *, int, void *);
extern float func_001F9B48(void *, void *);
extern int func_L00_002591D0(int *, int, int, int);
extern int func_L00_002592B8(int *, int, int, int);
extern void func_L00_002502F0(void *, int, int, int);
extern void func_L01_0026E0E0(int list, int state);
extern int func_001F9688(int);
extern int func_001F9770(void *);
extern unsigned char D_L15_001BB830[];
extern int D_0015ED84 MACRO_ADDR;
extern int D_L15_001BAAD0[];
extern char D_0014161B[];
extern char D_0013E533[];

/* FUN_L15_002e5958 -- src/overlays/shared/vendor_002D7C00.c (functional C for the port, not a match)
 * Update of a ring moby (class 1209, levels 15 and 17), one of a group (list moby[0x21]): state 0
 * measures the trigger radius and, for the group's first ring, links the other 0x4B9 rings to it and
 * counts them; in states 1-2, passing close in front of the ring lights it (sound 3, colour) and
 * advances the leader's count and countdown; all lit completes the group (list state 4, save bit set,
 * state 4 blinks), the countdown running out resets it (state 3).
 * From the staged near miss (nonmatching/shared/FUN_L15_002e5958.c); fixed: the matrix from
 * func_001FA2D8 is 4x4 (the near miss gave it a 4-float buffer, so the call overran the frame).
 * equiv: DIFFERENT only by the operand order of one commutative add (table base + index, retail
 * `addu base, index`, this `addu index, base`); every other operation matches. */
void FUN_L15_002e5958(unsigned char *moby) {
    int flag = 0;
    unsigned char *d = *(unsigned char **)(moby + 0x78);
    float v0[4];
    float v10[4][4];
    int a50[4];
    unsigned short s;
    int lk;
    int bit;
    int cond;
    int x;
    int y;
    int z;

    if ((unsigned)((unsigned char)moby[0x20] - 1) < 2) {
        func_001F9A28(v0, moby + 0x10, D_0013E533 + 0xE9D);
        if (func_001F9AF0(v0) < *(float *)(d + 0x14)) {
            func_001FA2D8(v10, moby + 0xC0);
            func_001F9D20(v0, v0, v10);
            if (v0[1] < 0.2f) flag = 1;
        }
    }

    switch ((unsigned char)moby[0x20]) {
    case 0:
        if (d[9] != 0) {
            *(short *)(d + 6) = 0;
        } else {
            func_L00_0024F7C8(moby, 0, v0);
            *(float *)(d + 0x14) = func_001F9B48(moby + 0x10, v0);
            d[0xB] = 0;
            *(unsigned char **)(d + 0x10) = moby;
            func_L00_002591D0(a50, moby[0x21], 0, 0);
            if (a50[0] != 0) {
                do {
                    unsigned char *p = (unsigned char *)a50[0];
                    if (p != moby && *(short *)(p + 0xA6) == 0x4B9) {
                        unsigned char *q = *(unsigned char **)(p + 0x78);
                        *(unsigned char **)(q + 0x10) = moby;
                        q[9] = 1;
                        *(float *)(q + 0x14) = *(float *)(d + 0x14);
                    }
                    d[0xB] = d[0xB] + 1;
                    func_L00_002592B8(a50, a50[0], 0, 0);
                } while (a50[0] != 0);
            }
            *(short *)(d + 6) = 0;
        }
        *(short *)d = 0;
        *(short *)(d + 2) = 0;
        d[0xA] = 0;
        func_L00_002502F0(moby, 0, 0, 0);
        *(short *)(d + 4) = 0;
        moby[0x20] = 1;
        return;
    case 1:
        goto f04;
    case 2:
    f04:
        s = *(unsigned short *)(moby + 0xB2);
        { unsigned char *t = D_L15_001BB830 + (short)s; lk = t[0x454]; }
        bit = 0;
        if (lk == 0) {
            bit = (*(int *)(D_0014161B + 0xAB75 + ((((short)s) >> 5) << 2) + (D_0015ED84 << 8)) >> (s & 0x1F)) & 1;
        }
        if ((lk != 0 || bit) && *(int *)(d + 0x1C) != 0) {
            func_L01_0026E0E0(moby[0x21], 4);
            moby[0x20] = 4;
            *(unsigned short *)d = *(unsigned short *)(d + 0x18);
            return;
        }
        cond = *(short *)(d + 4);
        if (cond == 0) if (flag) {
            unsigned char *p = *(unsigned char **)(d + 0x10);
            unsigned char *q = *(unsigned char **)(p + 0x78);
            if (p[0x20] == 1) {
                d[0xA] = 0;
                p[0x20] = 2;
                if (*(int *)(q + 0xC) > 0) {
                    *(short *)q = func_001F9688(*(int *)(d + 0xC));
                } else {
                    *(short *)q = 0x384;
                }
            }
            q[0xA] = q[0xA] + 1;
            func_0022DA68(3, 0x31, (int)moby);
            func_L00_002502F0(moby, 0, 0xFF, 0);
            *(short *)(d + 4) = 1;
        }
        if (moby[0x20] == 2) {
            if (d[0xA] >= d[0xB]) {
                func_L01_0026E0E0(moby[0x21], 4);
                func_0022DA68(1, 0x31, (int)moby);
                *(unsigned short *)d = *(unsigned short *)(d + 0x18);
                moby[0x20] = 4;
            } else {
                if (func_001F9770(d)) {
                    *(short *)(d + 4) = 0;
                    d[0xA] = 0;
                    func_0022DA68(2, 0x31, (int)moby);
                    func_L01_0026E0E0(moby[0x21], 3);
                    return;
                }
                if (func_001F9770(d + 2) == 0) return;
                func_0022DA68(0, 0x31, (int)moby);
                x = func_001F9688(0x2D);
                y = func_001F9688(5);
                z = func_001F9688(0x28);
                y += *(short *)d / z;
                if (x < y) {
                    *(short *)(d + 2) = func_001F9688(0x2D);
                    return;
                }
                y = func_001F9688(5);
                z = func_001F9688(0x28);
                *(short *)(d + 2) = y + *(short *)d / z;
                return;
            }
        }
        return;
    case 3:
        func_L00_002502F0(moby, 0, 0, 0);
        d[0xA] = 0;
        *(short *)(d + 4) = 0;
        moby[0x20] = 1;
        return;
    case 4:
        if (func_001F9770(d + 6) != 0) {
            *(short *)(d + 6) = func_001F9688(0x14);
            if (d[8] != 0) {
                func_L00_002502F0(moby, 0, 0xFF, 0);
                d[8] = 0;
            } else {
                func_L00_002502F0(moby, 0xFF, 0xFF, 0xFF);
                d[8] = 1;
            }
        }
        s = *(unsigned short *)(moby + 0xB2);
        *(int *)(D_0014161B + 0xAB75 + ((((short)s) >> 5) << 2) + (D_0015ED84 << 8)) |= (1 << (s & 0x1F));
        s = *(unsigned short *)(moby + 0xB2);
        *(int *)((unsigned char *)D_L15_001BAAD0 + ((((short)s) >> 5) << 2)) |= (1 << (s & 0x1F));
        return;
    default:
        return;
    }
}
