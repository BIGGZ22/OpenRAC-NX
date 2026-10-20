#include "common.h"
#include "structs.h"

typedef void (*BinFn)(void);

struct Chunk {
    unsigned char *dst;
    int size;
    int pad8;
    BinFn id;
};

extern int *D_001A7288 MACRO_ADDR;

/* rac1-decomp src/core/boot.c: func_0012DA38, `ParseBin` of boot.cpp
   (config/datamine/ntsc-u-rac1-matches.tsv pairs 0x00131CB8 with rac1's
   boot 0x0012DA38). Copy each boot-binary chunk to its destination while
   the chunk ids agree and return the id (0 when the first chunk carried
   none).

   Its own unit: rac1's config/core_text.objects gives ParseBin its own
   `boot.o` next to `crt0.o`, both built with the game compiler rather
   than `ee29`, so this file keeps the default profile while the libcdvd
   neighbours in 00131780.c need `sdk`. */
BinFn func_00131CB8(void) {
    unsigned char *p;
    struct Chunk *c;
    unsigned char *dst;
    unsigned char *src;
    BinFn id;

    id = 0;
    p = (unsigned char *)(D_001A7288[0] + (int)D_001A7288);
    while (c = (struct Chunk *)p, p = (unsigned char *)(c + 1), dst = c->dst, src = p,
           (id == 0) ? (id = c->id, 1) : (id == c->id)) {
        if (((c->size & 7) == 0) && (((unsigned int)p & 7) == 0) && (((unsigned int)dst & 7) == 0)) {
            unsigned char *d = dst;
            unsigned char *s = p;
            unsigned char *e = dst + c->size;
            while (d != e) {
                *(long *)d = *(long *)s;
                d += 8;
                s += 8;
            }
        } else {
            unsigned char *e = dst + c->size;
            while (dst != e) {
                *(int *)dst = *(int *)src;
                dst += 4;
                src += 4;
            }
        }
        p += c->size;
    }
    return id;
}
