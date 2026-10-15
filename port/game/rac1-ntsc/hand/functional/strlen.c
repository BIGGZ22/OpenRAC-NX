/* Carried from the PAL decompilation's nonmatching/functional/func_00116810.c (games/rac1) through the version
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
/* functional: medium. Plain byte-wise strlen. Retail (MMI quadword code) returns the same count, but it reads aligned 8/16-byte blocks, so its read pattern differs from this loop; results match for every valid NUL-terminated string. */
unsigned int strlen(const char *s) {
    const char *p = s;
    while (*p != 0) {
        p++;
    }
    return (unsigned int)(p - s);
}
