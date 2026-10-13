/* Carried from the PAL decompilation's nonmatching/functional/func_00115248.c (games/rac1/pal) through the version
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
/* functional: high. memcpy: 32-byte and 8-byte moves when dest and src are both 16-byte aligned and n >= 32, then byte moves for the rest; returns dest. Reads and writes are the same bytes in the same order as the assembly; counting is unsigned, so n >= 2^31 is handled too. */
typedef int u128_15248_cmemcpy __attribute__((mode(TI)));

void *memcpy(void *dest, const void *src, unsigned int n) {
    char *ret = dest;
    char *d = dest;
    const char *s = src;
    u128_15248_cmemcpy a;
    u128_15248_cmemcpy b;
    if (n >= 0x20 && (((unsigned long)s | (unsigned long)d) & 0xF) == 0) {
        do {
            a = *(u128_15248_cmemcpy *)s;
            n -= 0x20;
            s += 0x10;
            *(u128_15248_cmemcpy *)d = a;
            d += 0x10;
            b = *(u128_15248_cmemcpy *)s;
            s += 0x10;
            *(u128_15248_cmemcpy *)d = b;
            d += 0x10;
        } while (n >= 0x20);
        if (n >= 8) {
            do {
                long v = *(long *)s;
                n -= 8;
                s += 8;
                *(long *)d = v;
                d += 8;
            } while (n >= 8);
        }
    }
    while (n != 0) {
        *d++ = *s++;
        n--;
    }
    return ret;
}
