/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). _exit: ignores its argument and calls func_0011DDA0 with 0; retail tail-jumps there with a0 = 0, which is the same work since the function returns void. */
extern void func_0011DDA0(int);

void func_0012DA28(int code) {
    func_0011DDA0(0);
}
