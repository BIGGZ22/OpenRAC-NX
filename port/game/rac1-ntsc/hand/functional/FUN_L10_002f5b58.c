/* Carried from the PAL decompilation's nonmatching/shared/func_L10_002F6F18.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L10_002f5b58 -- src/overlays/shared/vendor_00299AF0.c
 * Best so far: SIZE ours 968 / retail 960, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Camera 19 activate (960 bytes): zeroes the path table at P+0x58/0x68, makes two passes of distances over the l
 *   Left to fix: retail keeps cam in $s7 and spills m to 0($sp); ours does the reverse and uses blezl where retail
 */
extern int func_001F9688(int);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B48(void *, void *);
extern float func_001FA6C0(int);
extern void func_001F99F8(void *);
extern void func_L00_002223F8(int, int);
extern char *D_L10_0015EF50 MACRO_ADDR;
extern char *D_L10_001B0930[];
extern char D_L10_0016CEC0[];
extern char D_L10_00167100[];

// Camera 19 activate: builds the two path length tables and copies the start state.
void FUN_L10_002f5b58(char *cam) {
    char *m;
    char *q;
    char *tb;
    char *fb;
    char *list;
    char *p;
    char *base;
    char *src;
    char *c10;
    char *c20;
    char *c30;
    float *fa;
    short *sa;
    float d, f;
    int i, j, k, off, idx;

    m = *(char **)(D_L10_0015EF50 + *(short *)(cam + 0x84) * 0x20 + 0x1C);
    m[0x38] = 1;
    q = *(char **)(cam + 0x70) + 0x40;
    *(int *)q = 0;
    *(int *)(q + 0x4) = 0;
    *(int *)(q + 0x8) = 0;
    *(int *)(q + 0xC) = 0;
    *(int *)(q + 0x10) = 0;
    *(short *)(q + 0x14) = 0;
    *(short *)(q + 0x16) = func_001F9688(*(short *)(m + 0x3A));

    tb = *(char **)(cam + 0x70) + 0x58;
    fb = *(char **)(cam + 0x70) + 0x68;
    {
        int *w = (int *)fb;
        short *z = (short *)tb;
        int n8;
        for (n8 = 7; n8 >= 0; n8--) {
            *w++ = 0;
            *z++ = -1;
        }
    }

    c30 = cam + 0x30;
    c10 = cam + 0x10;
    c20 = cam + 0x20;
    f = func_L00_001FF8B0(1.0f, *(float *)(D_L10_0016CEC0 + 0xB0));
    *(float *)(tb + 0x30) = f + f;

    idx = *(int *)(m + 0x20);
    if (idx >= 0) {
        list = D_L10_001B0930[idx];
        if (*(int *)list > 0) {
            fa = (float *)fb;
            sa = (short *)tb;
            for (i = 0; i < *(int *)list; i++) {
                off = i * 16;
                f = *(float *)(list + off + 0x1C);
                if (0.0f < f) {
                    *fa = f;
                    if (180.0f < f) {
                        *fa = *(float *)(tb + 0x30) * 57.29578f;
                    }
                    *sa = i;
                    fa++;
                    sa++;
                }
                k = (i + 1) % *(int *)list;
                d = func_001F9B48(list + (off + 0x10), list + (k * 16 + 0x10));
                *(float *)(list + off + 0x1C) = d;
                if (i < *(int *)list - 1) {
                    *(float *)(q + 0x8) = *(float *)(q + 0x8) + d;
                }
            }
        }
    }

    if (*(short *)(m + 0x3A) > 0) {
        d = *(float *)(q + 0x8) / func_001FA6C0(func_001F9688(*(short *)(m + 0x3A)));
        *(float *)(m + 0x3C) = d * 60.0f;
    }

    idx = *(int *)(m + 0x24);
    if (idx >= 0) {
        list = D_L10_001B0930[idx];
        if (*(int *)list > 0) {
            for (i = 0; i < *(int *)list; i++) {
                off = i * 16;
                k = (i + 1) % *(int *)list;
                d = func_001F9B48(list + (off + 0x10), list + (k * 16 + 0x10));
                *(float *)(list + off + 0x1C) = d;
                if (i < *(int *)list - 1) {
                    *(float *)(q + 0xC) = *(float *)(q + 0xC) + d;
                }
            }
        }
    }

    p = *(char **)(cam + 0x70);
    *(float *)(p + 0x0) = 0.03f;
    *(float *)(p + 0x4) = 0.2f;
    func_001F99F8(p + 0x10);
    p = *(char **)(cam + 0x70);
    *(float *)(p + 0x20) = 0.03f;
    *(float *)(p + 0x24) = 0.2f;
    func_001F99F8(p + 0x30);

    base = D_L10_00167100;
    src = *(char **)(base + 0x184);
    c30 = cam + 0x30;
    qcopy(c30, src + 0x30);
    qcopy(cam, src);
    qcopy(c10, src + 0x10);
    qcopy(c20, src + 0x20);

    if (*(int *)(m + 0x4C) == 0) {
        func_L00_002223F8(0x72, 1);
    }
}
