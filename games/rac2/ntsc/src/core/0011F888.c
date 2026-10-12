#include "common.h"
#include "structs.h"

extern void r1_func_00118CF0(void *);

extern void r1_func_00118CE0(void *);

extern int func_0011F8D0(void);

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
int func_0011F888(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

INCLUDE_ASM("asm/core", func_0011F8C0);

/* rac1-decomp src/core/0011DBE8.c: func_0011DC50 */
/* Reads a word through r1_func_00118CF0, rewrites its 13..15 bit field to 1,
   pushes it back, re-reads it and restores the original. Returns whether
   the field read back as 0. `srl` (not `sra`) at the end is the tell that
   the scratch word is unsigned. */
int func_0011F8D0(void) {
    unsigned int saved;
    unsigned int cur;
    r1_func_00118CF0(&saved);
    cur = (saved & 0xFFFF1FFF) | 0x2000;
    r1_func_00118CE0(&cur);
    r1_func_00118CF0(&cur);
    r1_func_00118CE0(&saved);
    return ((cur >> 13) & 7) == 0;
}
