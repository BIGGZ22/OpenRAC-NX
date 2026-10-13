/* Carried from the PAL decompilation's nonmatching/core/func_001153FC.c (games/rac1/pal) through the version
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
/* memset: stores the byte a1 into the n bytes at a0 and returns a0. The
   retail code stores 8 or 32 bytes at a time when the pointer is 16-byte
   aligned; the bytes written are the same. */
void *memset(void *p, int c, int n) {
    unsigned char *q = p;
    while (n-- > 0) {
        *q++ = (unsigned char)c;
    }
    return p;
}
