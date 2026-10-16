/* Carried from the PAL decompilation's nonmatching/core/func_001152F8.c (games/rac1) through the version
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
/* Copies n bytes from src to dst, safe when the two overlap: copies from the end when src is
   below dst and the source range reaches past dst, otherwise from the start. Returns dst. */
void *memmove(void *dst, const void *src, unsigned int n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    unsigned int i;

    if ((unsigned int)s < (unsigned int)d && (unsigned int)d < (unsigned int)s + n) {
        for (i = n; i != 0; i--) {
            d[i - 1] = s[i - 1];
        }
    } else {
        for (i = 0; i < n; i++) {
            d[i] = s[i];
        }
    }
    return dst;
}
