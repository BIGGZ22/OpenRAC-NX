/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void *func_L00_003B7410(char *a0, void *a1, void *a2, int a3)
{
    NODE *o = func_3100D8(0x88D);

    if (o != 0) {
        char *d = (char *)o + 0x10;
        void *s = o->f68;

        o->fB8 = (int)a0;
        o->f30 = 0xFF;
        o->fBC = a3;
        qcopy(d, a1);
        qcopy(s, a2);
        func_31A658(o, 2, -1);
    }
    return o;
}
