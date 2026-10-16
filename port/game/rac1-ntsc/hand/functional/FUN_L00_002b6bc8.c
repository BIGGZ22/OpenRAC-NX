/* Carried from the PAL decompilation's src/overlays/shared/vendor_002B33E8.c (matched C there) (games/rac1) through the version
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
extern int func_00213260(int);
extern unsigned char *func_L00_0025C338(void *);
extern float func_001F9B80(void *, void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_00259BC8(void *, int, int, void *, void *, float);
extern int func_L00_00257B90(int, int);
extern int func_001F9688(int);
extern char D_0013E533[];
extern char *D_L00_001ABA00[];
extern short D_L00_001B07B0[];
extern int D_L00_0015FFD8 MACRO_ADDR;
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) Vx_c2b6bc8;

// Finds mobys near a position that face it within a cone, claims them for this moby and returns how many.
int func_L00_002B7EC0_r(char *m, float r) __asm__("FUN_L00_002b6bc8");


int func_L00_002B7EC0_r(char *m, float r) {
    int count = 0;
    int *d;
    char *e;
    Vx_c2b6bc8 pos;
    Vx_c2b6bc8 q;
    unsigned char *c;
    int lim;
    int n;
    float dist;
    int i;
    *(Vx_c2b6bc8 *)&pos = *(Vx_c2b6bc8 *)(m + 0x10);
    d = *(int **)(m + 0x78);
    n = d[5];
    lim = func_00213260(n > 0 ? n : 1);
    d[5] = 0;
    for (i = 0; (e = D_L00_001ABA00[i]) != 0; i++) {
        c = func_L00_0025C338(e);
        if (c) {
            dist = func_001F9B80(&pos, e + 0x10);
            if (dist < 12.0f) {
                unsigned h = *(unsigned short *)(e + 0xA6);
                if ((h >= 0x1F4 && h <= 0x1F5) || (short)h == 0x1F9 || (short)h == 0x1FF) {
                    char *g = D_0013E533 + 0xE1D;
                    if (func_001FA688(*(float *)(g + 0x98), func_L00_001FF8B0(*(float *)(e + 0x10) - *(float *)(g + 0x80), *(float *)(e + 0x14) - *(float *)(g + 0x84))) < 0.9599311f) {
                        if (d[5] == lim) {
                            func_001F9A28(&q, e + 0x10, m + 0x10);
                            func_L00_001FF500(&q, &q, 1.5f);
                            func_L00_00259BC8(e, (int)m, 0x10000, g + 0x80, &q, 1.0f);
                            d[4] = func_001F9688(func_L00_00257B90(5, 0x2D));
                        }
                        d[5] = d[5] + 1;
                    }
                }
            }
            if (dist < r) {
                float f1;
                char *gb = D_0013E533 + 0xE1D;
                f1 = func_L00_001FF8B0(*(float *)(e + 0x10) - *(float *)(gb + 0x80), *(float *)(e + 0x14) - *(float *)(gb + 0x84));
                if (func_001FA688(*(float *)(gb + 0x98), f1) < 0.9599311f) {
                    char *h = *(char **)(c + 0x18);
                    if (h == 0) {
                        *(char **)(c + 0x18) = m;
                        count++;
                    } else if (dist < func_001F9B80(h + 0x10, e + 0x10)) {
                        *(char **)(c + 0x18) = m;
                        count++;
                    }
                }
            }
        }
    }
    for (i = 1; i <= D_L00_001B07B0[0]; i++) {
        char *mob = *(char **)&D_L00_0015FFD8 + (D_L00_001B07B0[i] << 8);
        if (mob) {
            if (mob[0x20] >= 0) {
                char *h2 = *(char **)(mob + 0x78);
                if (func_001F9B80(&pos, mob + 0x10) < r) {
                    char *g3 = D_0013E533 + 0xE1D;
                    if (func_001FA688(*(float *)(g3 + 0x98), func_L00_001FF8B0(*(float *)(mob + 0x10) - *(float *)(g3 + 0x80), *(float *)(mob + 0x14) - *(float *)(g3 + 0x84))) < 0.9599311f) {
                        *(char **)(h2 + 0x78) = m;
                        count++;
                    }
                }
            }
        }
    }
    return count;
}
