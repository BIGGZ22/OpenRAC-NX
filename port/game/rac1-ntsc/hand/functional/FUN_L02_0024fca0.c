/* Carried from the PAL decompilation's nonmatching/functional/func_L02_00250C78.c (games/rac1) through the version
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
/* functional: medium. Blends two RGBA images given as palette indices: out[k] = bytes of (int)(pal[a[k]]*(1-t) + pal[b[k]]*t) per channel (truncated, low byte kept), for 4*(size/4) pixels; the retail code stages the palette ($6, used for both tables) through the scratchpad with DMA and VU0, which this C does with local tables instead (not checked: DMA/scratchpad/D_PCR side effects, VU0 rounding). */
void FUN_L02_0024fca0(unsigned char *dst, unsigned char *idx_a, unsigned char *pal, unsigned char *idx_b, unsigned char *unused, int size, float t) {
    float table_a[256][4];
    float table_b[256][4];
    float w, ra, rb;
    int i, k, n, c;
    unsigned char *pa;
    unsigned char *pb;
    unsigned int *out;
    unsigned int px;
    int v;

    w = 1.0f - t;
    pa = pal;
    for (i = 0; i < 256; i++) {
        for (c = 0; c < 4; c++) {
            table_a[i][c] = (float)pal[i * 4 + c];
            table_b[i][c] = (float)pal[i * 4 + c];
        }
    }
    n = size >> 2;
    out = (unsigned int *)dst;
    pb = idx_b;
    for (k = 0; k < n * 4; k++) {
        px = 0;
        for (c = 0; c < 4; c++) {
            ra = table_a[idx_a[k]][c] * w;
            rb = table_b[pb[k]][c] * t;
            v = (int)(ra + rb);
            px |= (unsigned int)(v & 0xFF) << (c * 8);
        }
        out[k] = px;
    }
}
