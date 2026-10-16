/* Carried from the PAL decompilation's nonmatching/core/func_001166FC.c (games/rac1) through the version
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
/* strcpy: copies src to dst including the terminating NUL and returns dst.
   The aligned fast paths in the assembly copy only whole 8- or 16-byte blocks
   that hold no NUL; everything else goes byte by byte, so nothing past the NUL
   is written. */
char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++) != 0) {
    }
    return dst;
}
