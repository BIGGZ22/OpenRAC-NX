/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L07_003F8010(u8 *a0)
{
    u8 *s1 = *(u8 **)(a0 + 0x68);
    u8 *x = *(u8 **)(s1 + 0x410);
    u8 a[0x30];
    u8 b[0x10];

    if (x != 0) {
        qcopy(x + 0x10, a0 + 0x10);
        qcopy(x + 0xF0, a0 + 0xF0);
        func_002A93B0(x);
    }
    func_002A0F48(*(u8 **)(s1 + 0x410));
    if (*(u8 **)(s1 + 0x414) != 0) {
        func_0029FC20(a0, 0, a);
        func_00283678(*(u8 **)(s1 + 0x414) + 0xC0, a);
        qcopy(*(u8 **)(s1 + 0x414) + 0x10, b);
    }
    func_002A0F48(*(u8 **)(s1 + 0x414));
}
