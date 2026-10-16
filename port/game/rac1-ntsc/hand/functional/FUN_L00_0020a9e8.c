/* Carried from the PAL decompilation's src/overlays/shared/help_00203E98.c (matched C there) (games/rac1) through the version
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
extern s32 func_001F9688(s32);
extern unsigned char D_0013E533[] NOT_SDA;
extern f32 func_002132A8(f32 a, f32 b);
extern s32 func_L00_00257B90(s32, s32);
extern char D_L00_0017A680[];
extern int func_001F9740(int *);
extern float func_L00_00257C48(float, float);


/* NON_MATCHING FUN_L00_0020a9e8 -- src/overlays/shared/help_00203E98.c
 * Best so far: SIZE ours 680 / retail 684, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_0020a9e8: re-rolls four random motion records (shared help): for i in 0..3, if func_001F9740(&q[i]) t
 *   Best p5: 680 vs 684 bytes (q as a named int * with q[1], q[2], q[i]); p7 (same size) adds j=i+13 but registers
 */

/* Re-rolls four random motion parameters and copies each pair into its 0xB0-byte record. */
void FUN_L00_0020a9e8(void) {
    char *base = (char *)D_0013E533 + 0xE1D;
    int *q;
    char *g;
    int i, j;
    char *tbl;

    if (*(int *)(base + 0x2084) != 0) return;
    if (*(unsigned char *)(*(int *)(base + 0x2080) + 0x53) != 0) return;
    q = (int *)(base + 0x1060);
    g = base;
    tbl = D_L00_0017A680;
    for (i = 0; i < 4; i++) {
        char *f;
        if (func_001F9740(&q[i])) {
            if (i == 0) {
                int *s;
                *(float *)(g + 0x1028) = func_L00_00257C48(0.17453292f, 0.5235988f);
                s = (int *)(D_0013E533 + 0x1E7D);
                *s = func_L00_00257B90(func_001F9688(0x46), func_001F9688(0x96));
            } else if (i == 1) {
                *(float *)((char *)q - 0x28) = func_L00_00257C48(0.17453292f, 0.5235988f);
                q[1] = func_L00_00257B90(func_001F9688(0x28), func_001F9688(0x5A));
            } else if (i == 2) {
                *(float *)((char *)q - 0x18) = func_L00_00257C48(0.2617994f, 0.87266463f);
                q[2] = func_L00_00257B90(func_001F9688(0x28), func_001F9688(0x5A));
            } else {
                float old, nw;
                *(float *)(g + i * 0x10 + 0x1024) = func_002132A8(-0.08726646f, 0.5235988f);
                old = *(float *)(g + i * 0x10 + 0x1028);
                nw = func_L00_00257C48(0.34906584f, 0.95993108f);
                *(float *)(g + i * 0x10 + 0x1028) = nw;
                if ((old <= 0 && nw <= 0) || (old >= 0 && nw >= 0))
                    *(float *)(g + i * 0x10 + 0x1028) = -*(float *)(g + i * 0x10 + 0x1028);
                q[i] = func_L00_00257B90(func_001F9688(0x23), func_001F9688(0x46));
            }
        }
        f = g + i * 0x10;
        j = i + 13;
        base = (char *)(j * 0xB0);
        {
            char *r = tbl + (int)base;
            *(float *)(r + 0x68) = *(float *)(f + 0x1028);
            *(float *)(r + 0x64) = *(float *)(f + 0x1024);
        }
    }
}
