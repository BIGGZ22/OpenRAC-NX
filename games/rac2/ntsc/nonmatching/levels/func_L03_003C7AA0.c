/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L03_003C7AA0(char *a0, char *a1)
{
    char *a2 = *(char **)(a0 + 0x68);

    qcopy(a1, a0 + 0x10);
    *(f32 *)(a1 + 8) = *(f32 *)(a2 + 0xF8);
}
