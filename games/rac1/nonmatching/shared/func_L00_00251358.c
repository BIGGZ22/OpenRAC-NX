/* NON_MATCHING func_L00_00251358 -- src/overlays/shared/mobyfunc_0024FD50.c
 * Best so far: SIZE ours 44 / retail 48, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_00251358: unpacks three bytes (bits 32-39, 40-47, 48-55) of the 64-bit word at moby+0x38 into three i
 *   Wall: retail uses $at ($1) as an ordinary register for dsrl32 results, which gcc never allocates; the original
 */
/* unpacks three bytes from the upper half of the moby's 64-bit word at 0x38 */
void func_L00_00251358(char *m, int *a, int *b, int *c) {
    unsigned long v = *(unsigned long *)(m + 0x38);
    *a = (unsigned char)(v >> 32);
    *b = (unsigned char)(v >> 40);
    *c = (unsigned char)(v >> 48);
}
