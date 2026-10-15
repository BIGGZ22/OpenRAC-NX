/* Carried from the PAL decompilation's nonmatching/functional/func_00119EA8.c (games/rac1) through the version
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
/* functional: high. Same arithmetic as the assembly; shift counts are masked to 6 bits as dsllv/dsrlv do (the -1 count for exponent -1 is shift 63 in retail), and the result is the low 32 bits sign-extended. Only the tail's instruction order differs from retail. */
int ftoi(unsigned long x) {
    long e;
    int r;
    e = (x << 1) >> 53; e -= 0x433;
    if (e < -0x35) return 0;
    if (e >= 13) return 9999;
    x = (x << 12) >> 12; x |= 0x10000000000000UL;
    if (e < 0) {
        e = -e;
        x >>= (e - 2) & 63;
        if ((x & 3) == 3) x = (x >> 2) + 1;
        else x >>= 2;
    } else {
        x <<= e & 63;
    }
    r = (int)x;
    return r;
}
