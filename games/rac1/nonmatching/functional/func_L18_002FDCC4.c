/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Retail's loop over a u16 list, with the table base read once from D_L18_00160058 before the loop.
   The counter (the a3 argument, returned) is advanced as a 16-bit signed value when the record's halfword at +0xA6 is
   0x772 and its byte at +0x20 is not 8. Arguments a0 and a2 are not read by retail. Low risk: the signature is derived
   from the caller (which sets the count to 0 in its delay slot); the sign-extension of the counter is written as retail
   does it. */
extern u8 *D_L18_00160058 MACRO_ADDR;

s32 func_L18_002FDCC4(s32 a0, u16 *p, s32 a2, s32 count) {
    u8 *base = D_L18_00160058;
    u16 v;
    do {
        u8 *e;
        v = *p;
        e = base + ((v & 0x7FFF) << 8);
        if (*(s16 *)(e + 0xA6) == 0x772 && *(e + 0x20) != 8) {
            count = (s16)(count + 1);
        }
        p++;
    } while (!(v & 0x8000));
    return count;
}
