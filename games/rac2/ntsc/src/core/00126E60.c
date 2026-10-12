#include "common.h"
#include "structs.h"

extern void r1_func_0011A6C8();

extern void r1_func_0011A6C8(void *);

extern int func_001272A8(void *);

extern char r1_D_001534E0[];

extern void func_00126E60(void *);

extern int func_00126ED8(int);

extern int r1_D_00132E70[];

/*
 * sceDmaReset(mode): ret = D_CTRL.DMAE; for each of the ten channels
 * whose flag (r1_D_001534F8[i]) is set, zero chcr/tadr/madr/as1/as0/sadr of
 * the channel's registers (r1_D_00132E70[i]); D_STAT = 0xFF1F, then
 * D_STAT &= 0xFF1F0000; zero a 0x14-byte sceDmaEnv (func_00126F00) and
 * sceDmaPutEnv it (func_00127040); if mode == 1 set DMAE. Exact under
 * both compilers.
 *
 * Levers (build-sn/try/func_00126F60/RESULT.md):
 *  - an index loop: loop.c strength-reduces it into the two pointers and
 *    the down-counter itself, in retail's init order (the old
 *    hand-written two-pointer loop swapped them);
 *  - the MMIO spelled inline as `*(volatile unsigned int *)0x1000E0x0`
 *    (a pointer local becomes absolute `$at` addressing: the old 228);
 *  - the two D_STAT stores are not volatile, only its read, so the second
 *    store can go into the jal slot as in retail (a volatile store never
 *    does);
 *  - sceDmaPutEnv returns int: the `li 1` after it is in $v1.
 */
extern int r1_D_001534F8[];

extern int func_00127040(void *);

/* rac1-decomp src/core/00123168.c: func_00123208 */
/*
 * Wait for a DMA channel: spin while CHCR.STR (bit 8 of *arg0) is set;
 * after 0x1000000 polls report through r1_func_0011A6C8(r1_D_001534E0,
 * "libdma: sync timeout") and sceDmaPause (func_001272A8) the channel on
 * every further poll.
 *
 * The countdown starts at 0x1000000 and is decremented before the test:
 * retail's `lui 0x100; addiu -1` (splat's "D_FFFFFF") is that first
 * n-- folded into the constant, and reorg copies the loop's n-- into the
 * bnez slot. The old reading (0xFFFFFF, `if (n < 0)` then `n--`) gave
 * `lui 0xff; ori 0xffff` and was 24/112. The channel word is read
 * volatile, as the hardware register it is.
 */
void func_00126E60(void *arg0) {
    int n = 0x1000000;

    while ((*(volatile int *)arg0 & 0x100) != 0) {
        if (--n < 0) {
            r1_func_0011A6C8(r1_D_001534E0);
            func_001272A8(arg0);
        }
    }
}

INCLUDE_ASM("asm/core", func_00126ED0);

/* rac1-decomp src/core/00123168.c: func_00123280 */
int func_00126ED8(int arg0) {
    if ((unsigned int)arg0 >> 28 == 7) {
        arg0 &= 0x0FFFFFFF;
        arg0 |= 0x80000000;
    }
    return arg0;
}

/* rac1-decomp src/core/00123168.c: func_001232A8 */
/* Zero-fill n bytes at dst.

   The plain post-decrement loop: the counter becomes a new pseudo (n - 1
   in $v0, -1 in $v1) instead of the parameter decremented in place, and
   the empty delay slot is 2.9-ee itself -- it pads the short loop with
   nops in its own output and leaves the bne unfilled, so the assembler
   puts the nop in the slot. That is the "unfilled short-loop slot" seen
   elsewhere in core_text. Under 2.95.3 the same C gets the registers
   right but reorg fills the slot (6/52); the old do-while was 2/52. */
void func_00126F00(char *dst, int n) {
    while (n-- != 0) {
        *dst++ = 0;
    }
}

/* rac1-decomp src/core/00123168.c: func_001232E0 */
/* sceDmaGetChan: the channel's register block, 0 for an id past the ten
   channels. The in-range return comes first under 2.9-ee (the
   `>= 0xA` guard-first form matched 2.95.3 and is 21/40 here). */
int func_00126F38(unsigned int arg0) {
    if (arg0 < 0xA) {
        return r1_D_00132E70[arg0];
    }
    return 0;
}

/* rac1-decomp src/core/00123168.c: func_00123308 */
int func_00126F60(int arg0) {
    char buf[0x14];
    int flag;
    int i;

    flag = *(volatile unsigned int *)0x1000E000 & 1;

    for (i = 0; i < 10; i++) {
        if (r1_D_001534F8[i] != 0) {
            int *p = (int *)r1_D_00132E70[i];
            p[0] = 0;
            p[0xC] = 0;
            p[4] = 0;
            p[0x14] = 0;
            p[0x10] = 0;
            p[0x20] = 0;
        }
    }

    *(unsigned int *)0x1000E010 = 0xFF1F;
    *(unsigned int *)0x1000E010 = *(volatile unsigned int *)0x1000E010 & 0xFF1F0000;

    func_00126F00(buf, 0x14);
    func_00127040(buf);

    if (arg0 == 1) {
        *(volatile unsigned int *)0x1000E000 = *(volatile unsigned int *)0x1000E000 | 1;
    }
    return flag;
}
