/* functional: low. The blend matches the US VU0 code (qmtc2 + vitof12 of b2<<4 gives t = b2/256, w = 1 - t,
 * xyz = (1-t)*v1 + t*v2, w from v1; b2 == 0 copies v1). Unresolved: the US copies read their table from
 * each level's overlay (D_0017FEC0..D_001808C0), while this reads D_0019BDC0 of the main executable, so the
 * table is right only where that copy is the one running. */
/* Carried from the PAL decompilation's nonmatching/game/func_0020E360.c (games/rac1) through the version
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
/* Blends two 4-float entries of the table D_0019BDC0 (64-byte blocks, vector
   at +0x10 of each block). The 64-bit word at a0+0x38 holds three indices
   and a weight: b0 = bits 0..7 selects the first block, b1 = bits 8..15 the
   second, b2 = bits 16..23 is the weight (0..255, t = b2/256). Writes the
   blend (xyz) and the first entry's w to out. */
extern char D_0019BDC0[];

void FUN_0020d510(void *a0, void *out) {
    u64 packed = *(u64 *)((char *)a0 + 0x38);
    unsigned int b0 = packed & 0xFF;
    unsigned int b1 = (packed >> 8) & 0xFF;
    unsigned int b2 = (packed >> 16) & 0xFF;
    float *v1 = (float *)(D_0019BDC0 + (b0 << 6) + 0x10);
    float *dst = (float *)out;

    if (b2 == 0) {
        dst[0] = v1[0];
        dst[1] = v1[1];
        dst[2] = v1[2];
        dst[3] = v1[3];
        return;
    }

    {
        float *v2 = (float *)(D_0019BDC0 + (b1 << 6) + 0x10);
        float t = (float)b2 / 256.0f;
        float w = 1.0f - t;
        float p0 = v1[0] * w;
        float p1 = v1[1] * w;
        float p2 = v1[2] * w;

        dst[0] = p0 + v2[0] * t;
        dst[1] = p1 + v2[1] * t;
        dst[2] = p2 + v2[2] * t;
        dst[3] = v1[3];
    }
}
