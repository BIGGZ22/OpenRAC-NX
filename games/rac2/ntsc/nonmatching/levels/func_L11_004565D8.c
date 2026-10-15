/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L11_004565D8(char *p, char *a1)
{
    float dx = *(float *)(p + 0x10) - *(float *)(a1 + 0x0);
    float dz = *(float *)(p + 0x14) - *(float *)(a1 + 0x4);
    float ang = func_00283248(dx, dz);
    char *q;
    float amp;

    ang = ang + func_002A7878(-0.78539825f, 0.78539825f);
    amp = func_002A7878(-2.0f, 2.0f) + 8.0f;
    q = *(char **)(p + 0x68);
    qcopy(q + 0x230, a1);
    *(float *)(q + 0x230) = *(float *)(q + 0x230) + amp * func_00283180(ang);
    *(float *)(q + 0x234) = *(float *)(q + 0x234) + amp * func_00283198(ang);
}
