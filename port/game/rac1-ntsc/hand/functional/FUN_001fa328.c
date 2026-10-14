/* Carried from the PAL decompilation's nonmatching/game/func_001FA4F0.c (games/rac1/pal) through the version
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
/* out (3 columns of 4 floats) = A * B, where A is the 4x3 block at src (its three
   columns are vf4..vf6) and B the block at b (three columns vf1..vf3). Each lane:
   out[j][i] = (A0[i]*B[j][0] + A1[i]*B[j][1]) + A2[i]*B[j][2], as the VU0 does it. */
void FUN_001fa328(float *out, const float *src, const float *b) {
    int j, i;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 4; i++) {
            float t = src[i] * b[4 * j + 0];
            t = t + src[4 + i] * b[4 * j + 1];
            out[4 * j + i] = t + src[8 + i] * b[4 * j + 2];
        }
    }
}
