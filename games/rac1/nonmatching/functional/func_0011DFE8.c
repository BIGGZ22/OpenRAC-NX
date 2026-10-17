/* functional: high. 64-bit signed division (the compiler runtime's __divdi3, quotient only): retail takes the absolute values (flagging when exactly one operand is negative), runs the same 32-bit-limb long division as the unsigned routine, then negates the quotient when the signs differ; the result is exactly n / d truncated toward zero, with LONG_MIN handled as an unsigned magnitude. Retail executes break 7 for a zero divisor; the host would raise SIGFPE, so a zero divisor returns 0 instead (as the unsigned sibling does). */
long func_0011DFE8(long n, long d) {
    unsigned long un, ud, q;
    int neg = 0;

    if (d == 0) {
        return 0;
    }
    un = (unsigned long)n;
    ud = (unsigned long)d;
    if (n < 0) {
        un = 0 - un;
        neg = 1;
    }
    if (d < 0) {
        ud = 0 - ud;
        neg ^= 1;
    }
    q = un / ud;
    if (neg) {
        q = 0 - q;
    }
    return (long)q;
}
