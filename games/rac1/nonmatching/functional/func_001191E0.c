/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Cache maintenance only (index-invalidates the 128 sets of both ways of the instruction cache, skipping lines whose tag lies in [lo, hi]); the host has no such cache and the function has no memory, register-visible or return effect, so it is empty. */
void func_001191E0(unsigned int lo, unsigned int hi) {
    (void)lo;
    (void)hi;
}
