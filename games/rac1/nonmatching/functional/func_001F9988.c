/* functional: high. fcheck: unchecked. Busy-wait delay: spins about n / 2 times through six nops (n counts down by 2 while it is above 0) and has no other effect. The host needs no CPU-cycle delay, so the port does nothing. Only hand-written code calls it, with n in a0 as usual. Written from the asm. */
void func_001F9988(int n) {
    (void)n;
}
