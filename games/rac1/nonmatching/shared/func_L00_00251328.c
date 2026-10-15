/* NON_MATCHING func_L00_00251328 -- src/overlays/shared/mobyfunc_0024FD50.c
 * Best so far: SIZE ours 52 / retail 48, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_00251328: sets three bytes (bits 32-39, 40-47, 48-55) of the 64-bit word at moby+0x38 keeping the low
 *   Not matched (5 of 10 runs): retail does ld then dsll32/dsrl32 to clear the high half, and has a nop before the
 */
/* stores three bytes into the upper half of the moby's 64-bit word at 0x38 */
void func_L00_00251328(char *m, long a, long b, long c) {
    unsigned long v = *(unsigned long *)(m + 0x38);
    *(unsigned long *)(m + 0x38) = (unsigned int)v | (a << 32) | (b << 40) | (c << 48);
}
