/* Carried from the PAL decompilation's nonmatching/game/func_001F9F60.c (games/rac1) through the version
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
/* FastCountChars: sums a byte buffer two bytes per pass. The loop always runs
   at least once and keeps going while the count stays positive after each
   step of two. */
int ComputeByteStringHash(unsigned char *p, int n) {
    int sum = 0;
    do {
        int a = p[0];
        int b = p[1];
        p += 2;
        sum += a;
        n -= 2;
        sum += b;
    } while (n > 0);
    return sum;
}
