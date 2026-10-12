#include "common.h"
#include "structs.h"

extern void r1_func_0012C468(void *, void *);

extern int func_0012C680(void *, int);

extern int func_0012B8B0(void *, int);

extern int func_0012B1C0(void *);

extern int r1_func_001263A8(void *, int);

extern int func_0012B780(void *, int, int *, int *, void *);

extern int func_0012B568(void *);

extern char r1_D_00153948[];

extern int func_0012BB60(void *, int *, int *, int *, void *, void *, void *);

extern int r1_func_00125630(void *, int, int, int, int, void *, void *, void *);

extern char r1_D_00153968[];

/* rac1-decomp src/core/00125630.c: func_00127A90 */
/* slice0 (libmpeg.a:mpc.o): the per-picture macroblock loop -- sliceA0
 * (func_0012B780) parses the first slice header and starting mb address
 * (local "mbAddr"/"mbInc" pair, plus the zeroed 8-word block-info struct
 * at local0); then while mbAddr < mbCount: waitBdecOut (func_0012B1C0)
 * must succeed; if this macroblock doesn't already have a pending
 * mbAddressIncrement (mbInc==0, i.e. a fresh slice), peek 23 bits looking
 * for the next slice_start_code prefix, and if found read the next
 * mbAddressIncrement (func_0012B568) -- both faulting through +0x11C;
 * decode (decMB0, func_0012BB60) when mbInc==1, else skip (skipMB0,
 * func_0012BAA0) the run of skipped macroblocks; run motion compensation
 * (motionComp0, r1_func_00125630) on the result; for every macroblock after
 * the first, run the previous macroblock's IPU motion-comp dispatch
 * (doMC, r1_func_001263A8) using the alternating +0x810 buffer index before
 * flipping it; advance mbAddr and consume one mbInc, looping until the
 * picture's macroblocks are exhausted (return 0) or a fault (1/2/3)
 * happens along the way.
 */
int func_0012B8B0(void *arg0, int mbCount) {
    char *s = (char *)arg0;
    int mbAddr = 0;
    int mbInc = 0;
    int local48, local4C, local50;
    int local0[8];
    int scratch20[4];
    int scratch30[4];
    int r;

    r = func_0012B780(s, mbCount, &mbAddr, &mbInc, local0);
    if (r != 0) {
        return r;
    }
    *(int *)(s + 0x11C) = 0;

    for (;;) {
        int mbIdx;
        char *entry;

        if (!(mbAddr < mbCount)) {
            return 0;
        }

        mbIdx = *(int *)(s + 0x810);
        entry = s + mbIdx * 0x140;
        *(int *)(entry + 0x6CC) = 0;
        r = func_0012B1C0(s);
        if (r == 0) {
            return 2;
        }

        if (mbInc == 0) {
            int peeked = func_0012C680(s, 0x17);
            if (peeked == 0) {
                *(int *)(s + 0x11C) = 0;
                return 3;
            }
            if (*(int *)(s + 0x11C) != 0) {
                *(int *)(s + 0x11C) = 0;
                return 3;
            }

            mbInc = func_0012B568(s);
            if (*(int *)(s + 0x11C) != 0) {
                *(int *)(s + 0x11C) = 0;
                return 1;
            }
        }

        if (!(mbAddr < mbCount)) {
            r1_func_0012C468(s, r1_D_00153948);
            return 2;
        }

        if (mbInc == 1) {
            r = func_0012BB60(s, &local48, &local4C, &local50, local0,
                               scratch20, scratch30);
            if (r == 0) {
                *(int *)(s + 0x11C) = 0;
                return 1;
            }
        } else {
            r = func_0012BAA0(s, local0, &local4C, scratch20, &local48);
            if (r == 0) {
                *(int *)(s + 0x11C) = 0;
                return 2;
            }
        }

        r = r1_func_00125630(s, mbAddr, mbInc, local48, local4C, local0,
                           scratch20, scratch30);
        if (r == 0) {
            *(int *)(s + 0x11C) = 0;
            return 2;
        }

        if (mbAddr != 0) {
            int fault = *(int *)(s + 0x810);
            r1_func_001263A8(s, fault ^ 1);
        }

        mbAddr = mbAddr + 1;
        *(int *)(s + 0x810) ^= 1;
        mbInc = mbInc - 1;
    }
}

/* rac1-decomp src/core/00125630.c: func_00127C80 */
/* arg0 is a moby/entity-ish struct: a table index at +0x810 selects a
 * +0x140-strided sub-entry whose +0x6CC field gets set, and several
 * other struct-pointer outputs get filled from +0x150/+0x174 flags.
 * Ends by clearing bit 0 of *a4, reporting through r1_func_0012C468 (an
 * already-matched "log to r1_D_00153968" family member) first when
 * +0x150 == 1. The r1_func_0012C468 call takes arg0 -- the moby struct,
 * still live in $4 -- not a4/s0, which is a common trap here. */
int func_0012BAA0(void *a0, int *a1, int *a2, int *a3, int *a4) {
    int idx;
    char *entry;
    int ret = 1;
    int c;
    int mask;

    idx = *(int *)((char *)a0 + 0x810);
    entry = (char *)a0 + idx * 0x140;
    *(int *)(entry + 0x6CC) = 1;
    *(int *)((char *)a0 + 0x1B0) = 1;

    if (*(int *)((char *)a0 + 0x150) == 2) {
        *(int *)((char *)a1 + 0x14) = 0;
        *(int *)((char *)a1 + 0x10) = 0;
        *(int *)((char *)a1 + 0x4) = 0;
        *(int *)((char *)a1 + 0x0) = 0;
    }

    if (*(int *)((char *)a0 + 0x174) == 3) {
        *a2 = 2;
    } else {
        *a2 = 1;
        c = (*(int *)((char *)a0 + 0x174) == 2);
        a3[1] = c;
        *a3 = c;
    }

    if (*(int *)((char *)a0 + 0x150) == 1) {
        r1_func_0012C468(a0, r1_D_00153968);
        ret = 0;
    }
    mask = *a4;
    mask &= ~1;
    *a4 = mask;
    return ret;
}
