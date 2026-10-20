/* 0 of 1 places match - last line: 11_joba: 112 bytes, expected 136.
   The semantics reproduce (bounds test on 0x27, the 0x14 split into the 0x210/0x260 index,
   the field68/field24 guards, the (a3 & 0x3FFF) + 0x7D0 store, the 0x2A0 flag, return 1) but
   the basic-block layout and the epilogue sharing do not: retail keeps THREE separate exits
   (an early `jr $ra; daddu $v0,$zero,$zero` at 0x445D34, a forward `jr $ra; daddu ...` at
   0x445DA8 for both later `return 0`s, and the `jr $ra; sh $a0,0x2A0($v1)` at 0x445DA0), where
   this cc1 merges the two `return 0` epilogues into one and branches back to it, and it lowers
   the `a1 < 0x14` split differently. Six words and the block order differ.
   Needs a spelling that keeps two distinct `return 0` epilogues alive.
   Below: the whole candidate (types as in src/levels/11_joba/L11_0043E1A0.c; `JobaCallSlotView`
   is already declared there with field24/field28/field68, and D_001A63A8 is 0x001A63A8). */

extern u8 D_001A63A8[];

s32 func_L11_00445D28(u8 *arg0, s32 a1)
{
    JobaCallSlotView *slot = (JobaCallSlotView *)D_001A63A8;
    s32 a3;
    u8 *a0;

    if (a1 >= 0x27)
        return 0;
    a0 = *(u8 **)(arg0 + 0x68);
    if (a1 < 0x14)
        a3 = *(s32 *)(a0 + a1 * 4 + 0x210);
    else
        a3 = *(s32 *)(a0 + (a1 - 0x14) * 4 + 0x260);
    if (slot->field68 != 0)
        return 0;
    if (slot->field24 != -1)
        return 0;
    slot->field24 = (a3 & 0x3FFF) + 0x7D0;
    slot->field28 = 0;
    *(u16 *)(a0 + a1 * 2 + 0x2A0) = 1;
    return 1;
}
