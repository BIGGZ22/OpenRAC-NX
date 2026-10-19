/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). 64-bit signed remainder (libgcc-style __moddi3): |n| % |d| computed with unsigned 64-bit division in place of retail's 32-bit-limb long division, then negated when n is negative (the sign of d does not matter); the same result for every input. Retail traps (break 7) on d == 0; here that case returns 0 instead of crashing the host. */
long func_0011E860(long n, long d) {
    unsigned long un = (n < 0) ? (0UL - (unsigned long)n) : (unsigned long)n;
    unsigned long ud = (d < 0) ? (0UL - (unsigned long)d) : (unsigned long)d;
    unsigned long r;
    if (ud == 0) {
        return 0;
    }
    r = un % ud;
    return (n < 0) ? (long)(0UL - r) : (long)r;
}
