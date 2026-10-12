/* Selects one of two measured value tables by index and arms the resident channel. */
void func_L16_00412BA8(u8 *arg, s32 idx)
{
    u8 *base = D_001A63A8;
    u8 *s0;
    s32 v;

    if (*(s32 *)(base + 0x68) != 0)
        return;
    s0 = *(u8 **)(arg + 0x68);
    if (*(s32 *)(base + 0x24) != -1)
        return;
    if (idx == *(s32 *)(s0 + 0x354))
        return;
    if (idx < 20)
        v = *(s32 *)(s0 + (idx << 2) + 0x280);
    else
        v = *(s32 *)(s0 + ((idx - 20) << 2) + 0x2D0);
    *(s32 *)(base + 0x28) = 0;
    *(s32 *)(base + 0x24) = (v & 0x3FFF) + 0x7D0;
    func_2A6170(D_L16_001B1888);
    *(u8 *)(s0 + idx + 0x310) = 1;
    *(s32 *)(s0 + 0x350) = 0x384;
    *(s32 *)(s0 + 0x354) = idx;
}
