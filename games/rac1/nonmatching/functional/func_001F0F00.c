/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Nothing differs in behaviour: the word at D_00194200+0x1C is tested for a sign bit before it is masked (the and in the branch's delay slot), so a negative word gives -1, a masked value of 31 gives -1, and anything else gives the low five bits. */
extern char D_00194200[];

int func_001F0F00(void) {
    int v = *(int *)(D_00194200 + 0x1C);

    if (v < 0) {
        return -1;
    }
    v &= 31;
    if (v == 31) {
        return -1;
    }
    return v;
}
