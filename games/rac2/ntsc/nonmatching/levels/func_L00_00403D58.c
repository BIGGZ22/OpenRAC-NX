/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L00_00403D58(char *arg)
{
    int *o = *(int **)(arg + 104);
    unsigned char state = *(unsigned char *)(arg + 32);
    TI128 tmp;

    switch (state) {
    case 0:
        arg[32] = 1;
    case 1:
        qcopy(&tmp, arg + 16);
        func_002EE108(arg + 16, o, arg + 16);
        func_0037F9A0(arg + 16, arg + 16);
        func_0037E1D8(o[7], o, 0);
        func_L00_00403F90(arg, &tmp);
        if (func_002EDD48((char *)o + 16) != 0)
            arg[32] = 2;
        break;
    case 2:
        func_0037E1B8(o[7]);
        func_00310440(arg);
        return;
    default:
        break;
    }
    func_002E6EF8(func_L00_00404168, arg);
}
