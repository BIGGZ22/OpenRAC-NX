/* Carried from the PAL decompilation's nonmatching/game/func_L00_00242120.c (games/rac1/pal) through the version
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
extern char D_L00_001E8640[];
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern int func_00120478(float);
extern int func_001E9730_s(void *, ...) __asm__("func_001E93B0");

/* Builds a table of up to 512 (value, key) pairs on the stack from n records of
   0x20 bytes (+0 int base, +4 int value, +0x20 / +0x40 ints), sorts it by key
   (largest first, a bubble sort), and prints each pair through func_001E93B0. */
void FUN_L00_00241788(char *p, int n) {
    struct { float f; float k; } e[512];
    int i, j, k, noswap;
    float t;

    for (i = 0; i < n; i++, p += 0x20) {
        e[i].f = (float)*(int *)(p + 4);
        if (*(int *)p == 0) {
            *(int *)&e[i].k = 0;
        } else {
            int v = *(int *)(p + 0x20);
            if (v == 0) {
                v = *(int *)(p + 0x40);
            }
            e[i].k = func_001FA6C0(v - *(int *)p) * 0.0009765625f;
            e[i + 1].f = -1.0f;
            *(int *)&e[i + 1].k = 0;
        }
    }

    do {
        noswap = 1;
        if (n > 0) {
            i = 0;
            do {
                j = i + 1;
                if (e[i].k < e[j].k) {
                    t = e[i].f;
                    e[i].f = e[j].f;
                    e[j].f = t;
                    t = e[i].k;
                    e[i].k = e[j].k;
                    e[j].k = t;
                    noswap = 0;
                }
                i = j;
            } while (i < n);
        }
    } while (noswap == 0);

    if (n > 0) {
        k = 0;
        do {
            int r1 = func_001FA6D0(e[k].f);
            int r2 = func_00120478(e[k].k);
            func_001E9730_s(D_L00_001E8640, r1, r2);
            k++;
        } while (k < n);
    }
}
