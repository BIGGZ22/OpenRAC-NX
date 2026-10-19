/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). The least of three signed ints (retail: two pminw on the low words). */
int func_001F9BA0(int a, int b, int c) {
    int m = a < b ? a : b;
    return m < c ? m : c;
}
