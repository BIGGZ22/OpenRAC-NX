/* Carried from the PAL decompilation's nonmatching/shared/func_L14_00303270.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L14_00301de8 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: SIZE ours 448 / retail 444, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Chain of vectors advanced per tick (func_001F9A10 loops), copies 4 indexed quads, then interpolates a value fr
 *   p4.c is 436 vs 444 bytes: remaining diffs are two small moves (retail `daddu $v0,$v1,$zero` in the j loop arou
 */
typedef int u128_03270_c301de8 __attribute__((mode(TI)));
extern void func_001F9A10(void *, void *, void *);
extern float func_001FA6C0(int);
extern int func_001FA6E0(int, int, float);
extern short D_L14_001620C4;
extern short D_L14_001620C8;
extern short D_L14_001620F4;
extern short D_L14_001620F0;
extern short D_L14_001620F8;

// advances a timed chain of vectors and picks an interpolation value
void FUN_L14_00301de8(char *moby) {
    char *d = *(char **)(moby + 0x78);
    if (*(short *)(d + 0x12) != 0) {
        int i;
        int j;
        int k;
        char *p;
        char *q;
        u128_03270_c301de8 *dst;
        unsigned char *idx = (unsigned char *)(d + 0x18);
        *(short *)(d + 0x12) = *(short *)(d + 0x12) - 1;
        p = d + 0x260;
        q = d + 0x60;
        for (i = 0x16; i >= 0; i--) {
            func_001F9A10(q, q, p);
            q += 0x10;
            p += 0x10;
        }
        j = 0;
        do {
            char *base = d + j * 16;
            char *a = base + 0x460;
            char *b = base + 0x560;
            j++;
            for (i = 2; i >= 0; i--) {
                func_001F9A10(a, a, b);
                a += 0x40;
                b += 0x40;
            }
        } while ((float)j < 4.0f);
        dst = (u128_03270_c301de8 *)(d + 0x420);
        for (k = 0; k < 4; k++) {
            *dst = ((u128_03270_c301de8 *)(d + 0x20))[idx[k]];
            dst++;
        }
    }
    {
        int h = *(short *)(d + 0x12);
        if (*(int *)&D_L14_001620C4 < h) {
            float f = func_001FA6C0(h - *(int *)&D_L14_001620C4) / func_001FA6C0(*(int *)(d + 0x620) - *(int *)&D_L14_001620C4);
            *(int *)(d + 0x14) = func_001FA6E0(*(int *)&D_L14_001620F4, *(int *)&D_L14_001620F0, f);
        } else if (*(int *)&D_L14_001620C8 < h) {
            float f = func_001FA6C0(h - *(int *)&D_L14_001620C8) / func_001FA6C0(*(int *)&D_L14_001620C4 - *(int *)&D_L14_001620C8);
            *(int *)(d + 0x14) = func_001FA6E0(*(int *)&D_L14_001620F8, *(int *)&D_L14_001620F4, f);
        } else {
            *(int *)(d + 0x14) = 0;
        }
    }
}
