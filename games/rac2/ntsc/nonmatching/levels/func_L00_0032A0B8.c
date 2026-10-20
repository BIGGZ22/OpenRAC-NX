/* Level table lookup, 27 places: the record at base + index * 0xA430 whose second word is the
 * key, returning a pointer to its third word. Closest candidate is 11 of 12 instructions exact
 * - all 27 places differ only at +0x10: retail adds the element address as base + index
 * (addu $v0,$a2,$a1) where this cc1 always emits index + base (addu $v0,$a1,$a2). Operand
 * reorder, s32/u8* casts, named base and entry locals, an all-integer spelling, the spellings of
 * nonmatching/levels/func_L00_0032A088.c (same table, +0x20), the -G8 profile and the SDK
 * compiler all produce the same single swap. */
typedef int s32;
typedef unsigned char u8;

extern u8 D_L00_00268E20[];

u8 *func_L00_0032A0B8(s32 a0, s32 a1)
{
    s32 idx = a1 * 0xA430;
    s32 *e = (s32 *)(D_L00_00268E20 + idx);

    if (e[1] != a0) {
        return 0;
    }
    return D_L00_00268E20 + 8 + idx;
}
