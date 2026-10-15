/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L19_00422178(char *a0, float *a1)
{
    float t = func_3183D8(0.0f, (*(float *)(a0 + 0x2C) / *(float *)(*(int *)(a0 + 0x24) + 0x24)) * 6.0f);
    float r = func_3183D8(-3.14159274f, 3.14159274f);

    qcopy(a1, a0 + 0x10);
    a1[0] = a1[0] + t * func_2EBF18(r);
    a1[1] = a1[1] + t * func_2EBF30(r);
}
