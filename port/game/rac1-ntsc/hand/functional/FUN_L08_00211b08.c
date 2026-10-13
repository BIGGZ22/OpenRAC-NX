/* Carried from the PAL decompilation's nonmatching/functional/func_L08_00211A38.c (games/rac1/pal) through the version
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
/* functional: high. Written from the assembly, not compiled. Same reads (*p, D_0015ED7C), same stores (0 or x - threshold, as an int or float), same return codes (-1 when the bit pattern of the original value is zero or negative, -2 when positive, 0 on the store path); the clamp test is an ordered compare, which is false for NaN exactly as c.le.s is. */
extern float D_0015ED7C MACRO_ADDR;

int FUN_L08_00211b08(float *p) {
    float x = p[0];
    float lim = D_0015ED7C;
    if (x <= lim) {
        int bits = *(int *)&x;
        *(int *)p = 0;
        return ((bits <= 0) ? 1 : 0) - 2;
    }
    p[0] = x - lim;
    return 0;
}
