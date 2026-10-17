/* functional: high. Cache maintenance only (index-invalidates the 128 sets of both ways of the instruction cache, skipping lines whose tag lies in [lo, hi]); the host has no such cache and the function has no memory, register-visible or return effect, so it is empty. */
void func_001191E0(unsigned int lo, unsigned int hi) {
    (void)lo;
    (void)hi;
}
