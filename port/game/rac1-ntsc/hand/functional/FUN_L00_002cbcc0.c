/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002CD110.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L00_002cbcc0 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: SIZE ours 688 / retail 680, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Clears 17 particle slots of the moby, then emits 5 sparks (func_L00_0026DD20) and 3 smoke puffs (func_L00_0026
 *   Best p2.c (688 vs 680 bytes): remaining diffs are the $a1=0x80 hoist before loop 3, f12/f13 setup order of the
 *   third func_002132A8 call, the final `if (rand(2)) n[3]=0x44` coming out as bnel instead of beqz+delay-slot
 *   li, and one extra saved-register store ordering. Zero V4 in loop 3 needed a compound literal into shared b.
 */
extern void func_L00_00267A08(void *);
extern float func_002132A8(float, float);
extern void func_L00_001FF500(void *, void *, float);
extern int func_001F9688(int);
extern int func_L00_00257B90(int, int);
extern char *func_L00_0026DD20(char *pos, char *vel, int c, int d, float f);
extern int func_00213260(int);
extern void *func_L00_0026D000(void *pos, int spd, void *pos2, int col, float a, float b, float c, float d);
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) W4_c2cbcc0;

// Clears the particle slots of a moby and emits a burst of sparks and smoke when requested.
void FUN_L00_002cbcc0(char *m, int flag) {
    int **p = (int **)(*(char **)(m + 0x78) + 0x2C);
    int i;
    int j;
    W4_c2cbcc0 b;
    for (i = 16; i >= 0; i--) {
        if (*p) func_L00_00267A08(*p);
        *p = 0;
        p++;
    }
    if (flag) {
        for (j = 4; j >= 0; j--) {
            W4_c2cbcc0 r = {0};
            r.x = func_002132A8(-1.0f, 1.0f);
            r.y = func_002132A8(-1.0f, 1.0f);
            r.z = func_002132A8(-1.0f, 1.0f);
            b = r;
            func_L00_001FF500(&r, &b, func_002132A8(D_0015ED6C * 3.0f, D_0015ED6C * 6.0f));
            func_L00_0026DD20(m + 0x10, (char *)&r, 0x7F2F4F6F, func_L00_00257B90(func_001F9688(10), func_001F9688(15)), 35000.0f);
        }
    }
    for (j = 2; j >= 0; j--) {
        int col = (func_L00_00257B90(0x20, 0x80) << 24) | 0x606060;
        int k = 1;
        char *s;
        char *n;
        if (func_00213260(2)) k = -1;
        b = (W4_c2cbcc0){0};
        b.z = 1.0f;
        b.w = 1.0f;
        b.z = func_002132A8(0.01f, 0.025f) * D_0015ED60;
        n = func_L00_0026D000(m + 0x10, k, &b, col, 0.2f, 1.0f, 1.02f, 140000.0f);
        if (n) {
            s = n + 0x20;
            *(short *)(n + 0xA) = func_001F9688(func_L00_00257B90(0x32, 0x50));
            *(int *)(s + 4) = 2;
            s[0xA] = col >> 24;
            s[0xB] = n[0xA];
            if (func_00213260(2)) n[3] = 0x44;
        }
    }
}
