/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L00_003F34D8(char *src, int *src4, char *dst16, int *dst4, int *count)
{
    int i = *count;

    if (i < 0x46) {
        qcopy(dst16 + i * 16, src);
        dst4[i] = *src4;
        (*count)++;
    }
}
