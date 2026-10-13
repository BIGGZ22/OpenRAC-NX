/* Creates the handle's 0x274 record on demand, or copies the source windows into it
   and follows the flag's polarity on the record's byte. */
void func_L16_004050D8(u8 *arg)
{
    u8 *s1 = *(u8 **)(arg + 0x68);
    u8 *p = *(u8 **)(s1 + 0x274);
    u8 *q;

    if (p == 0) {
        q = func_3098A0(0xECB);
        if (q == 0)
            return;
        *(u8 **)(s1 + 0x274) = q;
        func_3218B0(q, arg);
        *(u16 *)(*(u8 **)(s1 + 0x274) + 0x32) = *(u16 *)(arg + 0x32);
        func_313AF0(*(u8 **)(s1 + 0x274), 1, 0, 0);
        return;
    }
    qcopy(p + 0x10, arg + 0x10);
    qcopy(p + 0xF0, arg + 0xF0);
    if (*(u16 *)(arg + 0x34) & 1) {
        *(u16 *)(p + 0x34) |= 1;
        *(u8 *)(*(u8 **)(s1 + 0x274) + 0x31) = 0;
    } else {
        *(u16 *)(p + 0x34) &= ~1;
        *(u8 *)(*(u8 **)(s1 + 0x274) + 0x31) = 1;
    }
}
