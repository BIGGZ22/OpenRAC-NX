/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L05_00416E08(char *object)
{
    char *sub = *(char **)(object + 0x68);
    char *src = *(char **)(sub + 0x50);

    qcopy(object + 0x10, src + 0x10);
    switch (*(int *)(sub + 0x54)) {
    case 0:
        qcopy(object + 0xC0, src + 0xC0);
        qcopy(object + 0xD0, src + 0xD0);
        break;
    case 1:
        qcopy(object + 0xC0, src + 0xC0);
        func_3066A0(object + 0xD0, src + 0xD0, -1.0f);
        break;
    default:
        return;
    }
    qcopy(object + 0xE0, src + 0xE0);
}
