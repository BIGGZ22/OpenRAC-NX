/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Same two stores as retail, in retail's order: 6 at p - 0x6AF0, then 0 at p - 0x6AF0 + 4; the bytes differ only in address folding and the delay slot. */
void func_L00_00203F08(int a, int b, char *p) {
    *(int *)(p - 0x6AF0) = 6;
    *(int *)(p - 0x6AF0 + 4) = 0;
}
