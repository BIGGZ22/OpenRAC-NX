/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L00_00404DD8(char *arg, void *a1)
{
    if (*(float *)(arg + 16) < 2.0f || *(float *)(arg + 16) > 1021.0f ||
        *(float *)(arg + 20) < 2.0f || *(float *)(arg + 20) > 1021.0f ||
        *(float *)(arg + 24) < 2.0f || *(float *)(arg + 24) > 1021.0f) {
        func_00310440(arg);
        return;
    }
    {
        char *p = arg + 16;

        if (func_002DE4B0(a1, p, 0, arg, 0) != 0) {
            qcopy(p, &D_L00_001BF220);
            arg[32] = 2;
            func_0035F450(5, 0, arg, 3705);
        }
    }
}
