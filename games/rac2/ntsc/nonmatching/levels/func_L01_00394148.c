/* func_L01_00394148 (248 bytes, 4 places) - loop body byte-identical, the preamble order
 * is what differs.  Measured with scratch/wslcmp.py: size EXACT at 248 for four spellings,
 * first difference at +0x8 ("instruction at +0x8 differs").  check.py not run.
 *
 * Body, exact: bail out unless the word at 0x001A8F40 is 2 and the byte at 0x001A7A40 is
 * nonzero, then walk the pointer table at D_L01_001BD1C0+0x18C for `*(s16*)(p+0xAA)` equal
 * to 0x398 or 0x1BF; for those, skip when the byte at 0x001B1E61 is set, otherwise
 * func_29F978(p, 0, 0x001B1E60) and three stores of 2.1f (0x40066666) to 0x001B1E80/84/88.
 * The count at +0x44 is re-read on every iteration, and this cc1 reproduces the `bnel`
 * annulled slots that carry it - the whole loop from the `beqz` onward matches.
 * The residue is the preamble: retail materialises the base (`lui $v1,0x1C` /
 * `addiu $v1,$v1,-0x2E40`) only after the 0x001A8F40 load and keeps it in $s2, builds the
 * table cursor with a single `addiu $s1,$s2,0x18C`, and leaves a `nop` of loop alignment
 * before it; this cc1 loads the state into $a0 instead of $v1, builds the cursor through a
 * second register (`addiu $a1,$s2,0x18C` then `daddu $s1,$a1,$zero`), and orders the saves
 * the other way.  Same wall as func_L10_00425798: the preamble/prologue interleave.
 * Note: the base MUST be spelled as a symbol - with a literal, cc1 folds `base + 0x18C`
 * into a new constant and the cursor disappears.
 */
typedef unsigned char u8;
typedef signed short s16;

extern int func_0029F978(void *a0, int a1, void *a2);

extern u8 D_L01_001BD1C0[];

void func_L01_00394148(void) {
    u8 *base = D_L01_001BD1C0;
    u8 **table = (u8 **)(base + 0x18C);
    u8 *p;
    int i;

    if (*(int *)0x001A8F40 != 2)
        return;
    if (*(u8 *)0x001A7A40 == 0)
        return;
    for (i = 0; i < *(s16 *)(base + 0x44); i++) {
        p = table[i];
        if (*(s16 *)(p + 0xAA) == 0x398 || *(s16 *)(p + 0xAA) == 0x1BF) {
            if (*(u8 *)0x001B1E61 != 0)
                continue;
            func_0029F978(p, 0, (void *)0x001B1E60);
            *(float *)0x001B1E80 = 2.1f;
            *(float *)0x001B1E84 = 2.1f;
            *(float *)0x001B1E88 = 2.1f;
        }
    }
}
