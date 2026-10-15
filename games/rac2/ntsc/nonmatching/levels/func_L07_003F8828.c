/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L07_003F8828(u8 *a0, u8 *a1)
{
    u8 *s1 = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x95) == 0) {
        u8 *s0 = s1 + 0x400;

        func_00282CF0(s0, a1, *(u8 **)(s1 + 0x418) + 0x10);
        func_002AFDD8(*(u8 **)(s1 + 0x418), s0, s0, 0);
        return;
    }
    qcopy(s1 + 0x400, a1);
}
