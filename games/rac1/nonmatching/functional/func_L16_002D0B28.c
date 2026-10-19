/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Retail's loop over a list of u16 entries (the 0x60 tail at 0x2D0B60 is folded into the loop; it is
   its own piece in the catalogue). Each entry selects a 256-byte record at base; the function returns 0 on the first
   record whose halfword at +0xA6 is 0x222 and whose byte at +0xBC is set, and 1 at the list terminator (bit 15).
   The arguments a0 and a2 are not read by retail; they are kept so the signature matches the caller. */
s32 func_L16_002D0B28(s32 a0, u16 *p, s32 a2, u8 *base) {
    u16 v;
    do {
        u8 *e;
        v = *p;
        e = base + ((v & 0x7FFF) << 8);
        if (*(s16 *)(e + 0xA6) == 0x222 && *(e + 0xBC) != 0) {
            return 0;
        }
        p++;
    } while (!(v & 0x8000));
    return 1;
}
