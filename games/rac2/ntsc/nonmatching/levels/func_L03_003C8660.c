/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L03_003C8660(char *a0, char *a1)
{
    char *v1 = *(char **)(a0 + 0x68);

    *(s32 *)(v1 + 0xAC) = 0;
    *(s32 *)(v1 + 0xA0) = 0;
    qcopy(v1 + 0x80, a1);
    func_002A7750(a0, 3, -1);
}
