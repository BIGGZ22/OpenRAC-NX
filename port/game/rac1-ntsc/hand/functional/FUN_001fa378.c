/* Carried from the PAL decompilation's nonmatching/game/func_001FA540.c (games/rac1) through the version
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
/* FUN_001fa378(dst, m, src): four vectors out of four: dst = m * src, with m
   as four columns (VU0: column 0 times x, plus column 1 times y, plus column 2
   times z, plus column 3 times w, accumulated in ACC in that order). The
   columns are read before the first store, as the VU registers are. */
void FUN_001fa378(float *dst, float *m, float *src) {
    float c0[4];
    float c1[4];
    float c2[4];
    float c3[4];
    float *end = src + 16;
    int k;

    for (k = 0; k < 4; k++) {
        c0[k] = m[k];
        c1[k] = m[4 + k];
        c2[k] = m[8 + k];
        c3[k] = m[12 + k];
    }
    do {
        float x = src[0];
        float y = src[1];
        float z = src[2];
        float w = src[3];

        for (k = 0; k < 4; k++) {
            float a = c0[k] * x;

            a = a + c1[k] * y;
            a = a + c2[k] * z;
            dst[k] = a + c3[k] * w;
        }
        dst += 4;
        src += 4;
    } while (src != end);
}
