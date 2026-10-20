/* from src/levels/shared/L00_002F4400.c: clears bit 31 of 161 words spaced 0xC apart,
 * starting at the resident table D_0018C958 + 8. Best of 3 compiles: exact 60 bytes
 * and every instruction shape (lui 0x19 base, lui 0x7FFF/ori mask, 0xA0 countdown with
 * `bgez` and `addiu v1,v1,0xC` in the delay slot), but 8 of 15 words differ:
 * this cc1 gives the 0x7FFFFFFF mask $a0 and the counter $a1 where retail has the
 * mask in $a1 and the counter in $a0, and it folds the +8 into the %lo (retail
 * materialises the base into $v0 then `addiu v1,v0,0x8`), which leaves a nop in the
 * head. for/do-while, `char *` + 0xC, `int *` + 3 and the declaration order all give
 * the same body. Unblocked by the source shape that allocates the mask to $a1 first. */
extern char D_0018C958[];

void func_L00_002F45B8(void) {
    int *p = (int *)(D_0018C958 + 8);
    int i;
    for (i = 0xA0; i >= 0; i--) {
        *p &= 0x7FFFFFFF;
        p += 3;
    }
}
