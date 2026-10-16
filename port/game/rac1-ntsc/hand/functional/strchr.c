/* Carried from the PAL decompilation's nonmatching/core/func_00116428.c (games/rac1) through the version
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
/* strchr: the first byte of the string a0 equal to a1 (taken as an
   unsigned char), or 0 if there is none. A1 of 0 finds the terminator.
   Retail scans 8 or 16 bytes at a time with MMI; the result is the same. */
char *strchr(char *s, int c) {
    unsigned char uc = (unsigned char)c;
    for (;; s++) {
        if ((unsigned char)*s == uc) {
            return s;
        }
        if (*s == 0) {
            return 0;
        }
    }
}
