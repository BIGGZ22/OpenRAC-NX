#include "common.h"
#include "structs.h"

extern void r1_func_00118D80(int);

extern void r1_func_0011DDD0(int, int);

extern void r1_func_0011DDE0(void *, void *, int);

extern int r1_func_0011DE28(int);

extern int r1_D_00131358[];

extern char r1_D_00130BF0[];

extern char r1_D_00131330[];

typedef struct { int a; int b; } r1_D_00131358_pair;

/* rac1-decomp src/core/0011D0D0.c: func_0011D370 */
/*
 * A word-at-a-time copy of nbytes (rounded down to words); returns 0.
 *
 * Exact under 2.9-ee. Retail leaves the loop branch's delay slot empty,
 * and that is 2.9-ee itself: it pads the short loop with nops in its own
 * output and leaves the bnez unfilled (as in libdma's r1_func_001232A8).
 * Under 2.95.3 reorg fills the slot with the `addiu $4,$4,4` bump, 52
 * bytes against 56, which is what kept this a stub.
 */
int func_0011FA70(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

INCLUDE_ASM("asm/core", func_0011FAA8);

/* rac1-decomp src/core/0011DDD0.c: func_0011DE38 */
/*
 * Same-size near-miss (8/0xD4 bytes). Same shape as r1_func_0011D3C8:
 * guarded by a hardware status bit at 0x10001810 (skip everything if
 * already done), two (addr,value) pokes through r1_func_0011DDD0, two
 * r1_func_0011DDE0 block copies (to a fixed load address and to
 * scratchpad at 0x82000), an interrupt disable/enable bracket, a
 * third r1_func_0011DDD0 poke, then a 6-entry table loop where
 * r1_func_0011DE28 reads the entry's current value back before
 * r1_func_0011DDD0 rewrites it. All three callees are handwritten
 * syscall wrappers.
 */
void func_0011FAB8(void) {
    int *t;
    r1_D_00131358_pair *p;
    unsigned int i;
    int old;

    if (*(volatile unsigned int *)0x10001810 & 0x100) {
        return;
    }

    t = r1_D_00131358;
    r1_func_0011DDD0(t[0], t[1]);
    r1_func_0011DDE0((void *)0x80076000, r1_D_00130BF0, 0x740);
    r1_func_0011DDE0((void *)0x82000, r1_D_00131330, 0x28);
    r1_func_00118D80(0);
    r1_func_00118D80(2);
    r1_func_0011DDD0(t[2], t[3]);

    p = (r1_D_00131358_pair *)(t + 4);
    for (i = 2; i < 8; i++) {
        old = r1_func_0011DE28(p->a);
        r1_func_0011DDD0(p->a, old);
        p++;
    }
}

INCLUDE_ASM("asm/core", func_0011FB8C);

INCLUDE_ASM("asm/core", func_0011FC48);

INCLUDE_ASM("asm/core", func_0011FC68);

INCLUDE_ASM("asm/core", func_00120354);
