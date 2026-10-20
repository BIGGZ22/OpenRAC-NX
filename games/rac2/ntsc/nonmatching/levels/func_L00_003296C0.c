/* 96 bytes against retail's 112 (27 places): the closest candidate differs only in how often
 * the per-index table is re-materialised. Retail materialises the global twice (`lui $a1,0x1B;
 * lw $a1,0x1AFC($a1)` at +0x10, and again into $v1 at +0x40) and re-reads the block pointer
 * four times, once per store; this cc1 keeps the global in one register, folds the store into a
 * single address register and reuses it, and drops the `daddu $v0,$a1,$zero` / `daddu $t0,$a2,
 * $zero` argument copies at +0x04/+0x0C (the value argument also needs to be `s32`, not `u8`,
 * or the store gains an `andi $a2,$a2,0xFF`). Tried: one expression per statement (96 bytes),
 * one address local (96 bytes).
 *
 * What would unblock it: the measured nosda levier - two C names for the same address
 * (`ref_L00_001B1AFC_00` for the first group of stores, a second name for the last two), so the
 * address is re-materialised instead of kept. The body itself is settled: the count byte lives
 * at block +4 and picks the next eight-byte record, which stores the object pointer, the byte
 * value and a cleared halfword, then the count is bumped.
 */
typedef int s32;
typedef unsigned char u8;

extern u8 *ref_L00_001B1AFC_00[];

void func_L00_003296C0(char *object, s32 index, s32 value)
{
    s32 offset;

    if (object == 0)
        return;
    offset = *(u8 *)(ref_L00_001B1AFC_00[index] + 4) * 8;
    *(char **)(ref_L00_001B1AFC_00[index] + offset) = object;
    *(u8 *)(ref_L00_001B1AFC_00[index] + offset + 5) = value;
    *(unsigned short *)(ref_L00_001B1AFC_00[index] + offset + 6) = 0;
    ++*(u8 *)(ref_L00_001B1AFC_00[index] + 4);
}
