/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). 64-bit unsigned division (the compiler runtime's __udivdi3, quotient only): retail does a 32-bit-limb long division (normalise the divisor, estimate each half with divu, correct up to twice, final multiply check); the result is exactly n / d. Retail executes break 7 for a zero divisor; the host would raise SIGFPE, so a zero divisor returns 0 instead. */
unsigned long func_0011EF28(unsigned long n, unsigned long d) {
    if (d == 0) {
        return 0;
    }
    return n / d;
}
