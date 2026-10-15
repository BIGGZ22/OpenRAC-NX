/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L06_0045A0D0(char *a0, char *a1, float f, float g, float h)
{
    char buf[16];
    char *q = a1;

    if (*(float *)(*(char **)(a1 + 0x11C)) - 0.3f < *(float *)(a0 + 0x18)) {
        *(float *)(a1 + 0xC8) -= 0.163333335f;
    }
    qcopy(buf, a0 + 0x10);
    func_002AA548(buf + 8, q + 0xC8, *(float *)(q + 0x128), f, g, h);
    if (func_L04_0033CB08(a0, q + 0xC0, 0x400, 0x10, 1.0f, 0.0f) != 0) {
        func_00282D30(q + 0xC0, q + 0xC0, 0.5f);
    }
}
