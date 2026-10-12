#include "common.h"
#include "structs.h"

extern long r1_func_0012AAC8();

extern long r1_func_0012AAC8(void *, int);

/* The bitstream reader's state (func_0012E890 sets it up). */
typedef struct {
    unsigned long acc;     /* 0x00: the next bits, top-aligned */
    unsigned char *start0; /* 0x08 */
    unsigned char *cur;    /* 0x0C: next byte to load */
    unsigned int bits;     /* 0x10: valid bits in acc */
    long total;            /* 0x18: bits consumed so far */
    unsigned char *start;  /* 0x20: the ring buffer */
    unsigned char *end;    /* 0x24 */
    int len;               /* 0x28 */
} BitStream;

/* rac1-decomp src/core/0012A2F0.c: func_0012ABF8 */
/* _sysbitJump (libmpeg bit.c): skip n bytes, then refill through
   _sysbitFlush. Clearing acc and bits before the total update is the
   one statement order that matches. */
void func_0012EA18(void *arg0, int n) {
    BitStream *p = arg0;
    p->acc = 0;
    p->bits = 0;
    p->total += n * 8;
    p->cur = p->start0 + (p->total >> 3);
    if (p->cur >= p->end) p->cur -= p->len;
    r1_func_0012AAC8(p, 0);
}
