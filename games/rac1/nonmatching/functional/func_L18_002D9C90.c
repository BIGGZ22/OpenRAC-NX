/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Stores 3 into the byte at +0x20 of the first argument, stores the second and third arguments at
   +0x7C and +0x78 of the third (a pointer taken from the caller's object), and returns 1. No host-only adjustments. */
s32 func_L18_002D9C90(u8 *a0, u32 a1, u32 a2, u8 *a3) {
    *(a0 + 0x20) = 3;
    *(u32 *)(a3 + 0x78) = a2;
    *(u32 *)(a3 + 0x7C) = a1;
    return 1;
}
