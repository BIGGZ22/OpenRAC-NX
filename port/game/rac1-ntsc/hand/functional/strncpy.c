/* Carried from the PAL decompilation's nonmatching/core/func_00116B00.c (games/rac1/pal) through the version
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
/* newlib strncpy: copies up to n bytes from src to dst, stopping after a NUL
   that is copied; the rest of the n bytes are filled with NULs. Returns dst.
   Retail moves 16- and 8-byte blocks that contain no NUL (aligned pointers,
   enough bytes left); the bytes written are the same as the byte loop. */
void *strncpy(void *dst, const void *src, int n) {
    char *d = (char *)dst;
    const char *s = (const char *)src;
    unsigned int m = (unsigned int)n;

    while (m != 0) {
        char c = *s;
        s++;
        *d = c;
        d++;
        m--;
        if (c == 0) {
            break;
        }
    }
    while (m != 0) {
        *d = 0;
        d++;
        m--;
    }
    return dst;
}
