/* 0 of 1 places match - last line: 17_smolg: 80 bytes, expected 88
   Whether the shared block (D_00189E20) selects this object: the 0x229C == 3 or
   0x2294 == 0x1B pair opens the 0x588 slot; otherwise an idle 0x34E counter leaves
   the 0x33C slot. Retail builds the answer through a shared tail
   (`bne $v0,$a0, ->tail` with `daddu $v0,$zero,$zero` in the delay slot, and the
   two true paths putting `addiu $v0,$zero,0x1` in a delay slot) - i.e. an if/else
   tree, not a boolean expression. This cc1 if-converts the `||` chain into
   `beql` + `xor`/`sltiu` (80 bytes) and, with a result variable and a merged
   else, into 92. Blocker: the if/else tail shape. */
typedef char *ptr;
typedef short s16;
typedef int s32;
typedef unsigned char u8;

extern u8 D_00189E20[];

s32 func_L17_003C5E20(s32 a0)
{
    char *p = D_00189E20;

    if (*(s32 *)(p + 0x229C) == 3 || *(s32 *)(p + 0x2294) == 0x1B) {
        if (*(s32 *)(p + 0x588) == a0)
            return 1;
        return 0;
    }
    if (*(s16 *)(p + 0x34E) != 0)
        return 0;
    if (*(s32 *)(p + 0x33C) == a0)
        return 1;
    return 0;
}
