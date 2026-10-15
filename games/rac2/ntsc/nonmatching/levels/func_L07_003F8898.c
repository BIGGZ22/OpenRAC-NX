/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L07_003F8898(u8 *a0, u8 *a1)
{
    u8 *s0 = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x95) == 0) {
        func_00283098(a1, s0 + 0x400, *(u8 **)(s0 + 0x418) + 0xC0);
        func_00282CC0(a1, a1, *(u8 **)(s0 + 0x418) + 0x10);
        return;
    }
    qcopy(a1, s0 + 0x400);
}
