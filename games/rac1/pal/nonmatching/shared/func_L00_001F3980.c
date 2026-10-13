/* NON_MATCHING func_L00_001F3980 -- src/overlays/shared/effects_001EE2E0.c
 * Best so far: BYTES 1/8 (87.5% of the bytes match), checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Returns -1 (`jr $31` with `addi $2, $0, -0x1`, marked "handwritten instruction"). Stopped on the wall: retail 
 *   The handwritten marker suggests this was assembly in the original; there is no C form that matches. Nothing to
 */
/* Returns -1. */
int func_L00_001F3980(void) {
    return -1;
}
