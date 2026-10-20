#include "common.h"
#include "structs.h"

extern int r1_func_00118C70(void *);

extern int r1_D_00130420;

extern int r1_D_00130424;

extern void func_0011F640(void);

extern int r1_func_0011DA08();

extern void r1_func_0011DB98(int, int);

extern int r1_func_0011DA80(int, int, void *);

extern int r1_D_00130410[];

extern int r1_D_00130408;

extern void func_0011F718(void);

ASM_FUNC("asm/core", func_0011F5E0);

ASM_FUNC("asm/core", func_0011F628);

/* rac1-decomp src/core/0011D960.c: func_0011D9C0 */
void func_0011F640(void) {
    int a[8];
    int b[8];
    a[1] = 1;
    a[2] = 1;
    b[1] = 1;
    b[2] = 1;
    r1_D_00130420 = r1_func_00118C70(a);
    r1_D_00130424 = r1_func_00118C70(b);
}

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
int func_0011F688(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

/* rac1-decomp src/core/0011D960.c: func_0011DA40 */
/* kFindAddress (libkernl.a:initsys.o): scan the words from p up to, not
 * including, end for val; returns the matching address, or 0 if the scan
 * reaches end. */
int *func_0011F6C0(int *p, int *end, int val) {
    while (*p != val && p < end)
        p++;
    if (!(p < end))
        p = 0;
    return p;
}

ASM_FUNC("asm/core", func_0011F700);

INCLUDE_ASM("asm/core", func_0011F710);

/* rac1-decomp src/core/0011D960.c: func_0011DA98 */
/* InitSystemCallTableAddress (libkernl.a:initsys.o): patches two kernel
   entry points found by scanning for known function bodies
   (FindAddress = r1_func_0011DA80, matching kCopy = r1_func_0011DA08 and a
   second reference routine, func_0011F6C0) into the two descriptors of
   r1_D_00130410 (kFindAddress/r1_func_0011DB98, called once per descriptor),
   then records where the two scans converge (they start from the same
   base and offset by different constants, and are each advanced by 4
   bytes -- whichever trails -- until they land on the same address) in
   r1_D_00130408. `a`/`b` have to be unsigned: retail's convergence test is
   `sltu`. */
void func_0011F718(void) {
    unsigned int a, b;
    int p1, p2;

    r1_func_0011DB98(r1_D_00130410[0], r1_D_00130410[1]);
    r1_func_0011DB98(r1_D_00130410[2], r1_D_00130410[3]);

    p1 = r1_func_0011DA80(0x80000000, 0x80080000, (void *)func_0011F6C0);
    p2 = r1_func_0011DA80(0x80000000, 0x80080000, (void *)r1_func_0011DA08);
    a = p1 - 0x20C;
    b = p2 - 0x168;
    while (a != b) {
        if (a < b) {
            p1 = r1_func_0011DA80(p1 + 4, 0x80080000, (void *)func_0011F6C0);
            a = p1 - 0x20C;
        } else {
            p2 = r1_func_0011DA80(p2 + 4, 0x80080000, (void *)r1_func_0011DA08);
            b = p2 - 0x168;
        }
    }
    r1_D_00130408 = a;
}

ASM_FUNC("asm/core", func_0011F818);
