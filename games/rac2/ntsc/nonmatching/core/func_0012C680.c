/* from src/core/0012C508.c: C that compiles but does not link at its places yet */
/* rac1-decomp src/core/00125630.c: func_00128860 */
/*
 * REVERTED -- decode is complete and believed correct; blocked on
 * register allocation, and the overage is a SIZE mismatch (0x114 vs
 * retail's 0x108) that overflows .core_text into .core_data, so it
 * cannot be left in as documented-close.
 *
 * A bitstream refill-and-extract: if the cache is empty or holds fewer
 * than n bits, wait for the DMA channel to go idle, refill from the
 * table, then return the top n bits.
 *
 *   extern long r1_func_00128638_wide(void *, int *) __asm__("func_0012C458");
 *
 *   int func_0012C680(void *arg0, int n) {
 *       char *s = (char *)arg0;
 *       int bits;
 *       if (*(int *)(s + 0x818) != 0 || *(int *)(s + 0x83C) < n) {
 *           int *tbl = r1_D_00132F70;
 *           int counter = 0;
 *           while ((*(volatile int *)0x10002010 & 0x80004000) == 0x80000000) {
 *               if (counter++ >= 0x1389) {
 *                   r1_func_0012BCC8(*(int *)(s + 0x858));
 *                   counter = 0;
 *               }
 *           }
 *           *(volatile int *)0x10002000 = 0x40000000;
 *           *(int *)(s + 0x818) = tbl[4];
 *           *(int *)(s + 0x838) = (int)r1_func_00128638_wide(s, tbl);
 *           *(int *)(s + 0x83C) = 0x20;
 *       }
 *       bits = *(int *)(s + 0x838);
 *       return (unsigned int)bits >> -n;
 *   }
 *
 * Confirmed right along the way: `counter++ >= 0x1389` is correct, not a
 * violation of the don't-put-++-in-a-comparison lever -- retail carries
 * the `daddu $2,$6,$0` spare move that lever warns about, so here the
 * move is EVIDENCE the ++ belongs inside the comparison. The dsll32/dsra32
 * after the jal is the 64-bit-return narrowing, handled by the _wide alias.
 *
 * Why it failed under 2.95.3: retail keeps only $16/$17/$18 across the
 * call (object, r1_D_00132F70 base, n) in a 0x40 frame and REMATERIALISES
 * the hardware constants 0x10002010 / 0x80004000 / 0x80000000 with
 * lui/ori at each use, where 2.95.3 hoists all three into callee-saved
 * registers (a 0x70 frame, 0x114 against 0x108). 2.9-ee, this object's
 * compiler, rematerialises them like retail (see func_0012C788), and the
 * C above then comes out 268 against 264: the frame and the wait loop
 * are right, but n is parked in $a3 and negated early where retail
 * keeps it in $s2 and negates it at the end. Still open.
 */
/* _peepBit: return the next count bits of the IPU bit stream without
   consuming them, refilling the 32-bit window from the IPU (FDEC) when
   it runs short and nudging the callback while the IPU stays busy.
   Adapted from Lombyte (GPL-3.0) for PAL. */
int func_0012C680(void *arg, int count) {
    char *ctx = arg;
    int counter;

    if (*(int *)(ctx + 0x818) == 0 && *(int *)(ctx + 0x83C) >= count) {
        goto done;
    }
    counter = 0;
    if ((*(volatile unsigned int *)0x10002010 & 0x80004000) == 0x80000000) {
        do {
            if (counter++ >= 0x1389) {
                r1_func_0012BCC8(*(void **)(ctx + 0x858));
                counter = 0;
            }
        } while ((*(volatile unsigned int *)0x10002010 & 0x80004000) == 0x80000000);
    }
    *(volatile unsigned int *)0x10002000 = 0x40000000;
    *(int *)(ctx + 0x818) = r1_D_00132F70[4];
    *(int *)(ctx + 0x838) = func_0012C458(ctx);
    *(int *)(ctx + 0x83C) = 0x20;
done:
    return (unsigned int)*(int *)(ctx + 0x838) >> (0x20 - count);
}
