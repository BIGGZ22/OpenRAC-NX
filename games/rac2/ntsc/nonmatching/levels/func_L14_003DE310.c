/* Near miss from src/levels/14_aranos_prison/L14_00304190.c — 2 words (8 bytes) differ.
 *
 * Kept as a starting point; not linked, not counted.
 *
 * Retail is 80 bytes; the body below compiles to 80 bytes and matches instruction
 * for instruction except two words, both about how GCC picks a branch sense:
 *
 *   003DE330  bgez $v0, +0x1C      |  bltz  $v0, +0x1C      (delay slot differs too)
 *   003DE340  c.ole.s $f0, $f1    |  c.olt.s $f1, $f0      or c.ole.s, depending on wording
 *
 * Everything structural is settled and worth keeping:
 *   - the loop runs four times: `n` starts at 3, is decremented once per
 *     iteration, and `p++` plus the `n >= 0` test sit *after* the body. So it is a
 *     do/while, and the `addiu $a1,$a1,-1` in the `beqz` delay slot runs even on
 *     the null-slot exit.
 *   - retail's `bgez` skips straight to the loop test at 0x3DE350, so both inner
 *     tests jump to the same block rather than branching into the body.
 *   - the float test is `f <= 0.0f` and `bc1tl` (branch-likely, on true) skips the
 *     clear, so the slot is cleared when the value is greater than zero. Getting
 *     `> 0.0f` in the C is what produces `bc1tl`; getting `<= 0.0f` is what
 *     produces `c.ole.s $f0, $f1`. No single wording tried produces both.
 *
 * Nine wordings were tried: nested `if` with `< 0`, with `>= 0` and empty else
 * branches, a single `&&` chain, and the block moved in and out of the `e != 0`
 * guard. The `<= 0.0f`-with-empty-else form gives `c.ole.s` but `bc1fl`; the
 * `&&` chain gives `bc1tl` but `c.olt.s`.
 *
 * What is left is GCC's branch-sense and static-likelihood choice, which tracks
 * the exact block layout rather than the C. Not yet tried: making the s8 test the
 * loop's own `continue` condition, or spelling the float test as `!(x > 0.0f)`.
 */
void func_L14_003DE310(char *a0) {
    u32 *p = (u32 *)(*(u32 *)(a0 + 0x68) + 0xE0);
    s32 n = 3;
    do {
        u32 e = *p;
        n--;
        if (e != 0 && *(s8 *)(e + 0x20) < 0) {
            if (*(float *)(*(u32 *)(e + 0x68) + 0x40) > 0.0f)
                *p = 0;
        }
        p++;
    } while (n >= 0);
}