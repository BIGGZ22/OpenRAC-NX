/* func_L00_002D6418 (160 bytes, 27 places) -- NEAR MISS, not integrated.
   Decoded (00_aranos_tutorial) - a decision chain that returns 0 or 1:
     if (D_001A7908 == 0) return 0;
     v = <the word reached through $gp, offset -0x60F0>;
     if (v == 2) return 0;
     if (v == 6) return 0;
     if (D_001B8E10 != 0 && D_00190086 == 5) return 0;
     base = 0x00189E20;
     if (*(u8  *)(base + 0x22B4) != 0)   return 0;
     if (*(s32 *)(base + 0x229C) == 0x14) return 0;
     if (*(s32 *)(base + 0x24CC) == 0)   return 0;
     v = *(s32 *)(base + 0x2294);
     if (v == 0x6F || v == 0x31 || v == 0x1C || v == 0x8C) return 0;
     return 1;
   Two shape notes worth keeping: the `beql $v0,$zero` at +0x28 carries the *base setup*
   `lui $v0,0x19` in its delay slot (branch-likely executes the slot only when taken, so
   both paths into the body arrive with $v0 = 0x190000), and the first word is read through
   $gp (`lw $v1,-0x60F0($gp)`) while the same block is reached absolutely elsewhere - the
   sda/double-name recipe applies.

   Measured, three spellings (the $gp word under an `__attribute__((sda))` name plus an
   absolute `_1` alias for the block, the block inline, and the block through a base local):
   168/168/172 bytes against 160, 35-41 diffs. The chain's branch layout is what differs
   first, so the decode above is the useful part. 27 places - the widest row in the lot. */

typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern s32 D_L00_001A7908;
extern s32 D_L00_001B8E10;
extern s16 D_L00_00190086;
extern s32 D_L00_00189E20 __attribute__((sda));
extern u8 D_L00_00189E20_1[];

s32 func_L00_002D6418(void)
{
    u8 *blk = D_L00_00189E20_1;
    s32 v;

    if (D_L00_001A7908 == 0)
        return 0;
    v = D_L00_00189E20;
    if (v == 2)
        return 0;
    if (v == 6)
        return 0;
    if (D_L00_001B8E10 != 0 && D_L00_00190086 == 5)
        return 0;
    if (*(u8 *)(blk + 0x22B4) != 0)
        return 0;
    if (*(s32 *)(blk + 0x229C) == 0x14)
        return 0;
    if (*(s32 *)(blk + 0x24CC) == 0)
        return 0;
    v = *(s32 *)(blk + 0x2294);
    if (v == 0x6F || v == 0x31 || v == 0x1C || v == 0x8C)
        return 0;
    return 1;
}
