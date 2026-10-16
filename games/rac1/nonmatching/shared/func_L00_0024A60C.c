/* NON_MATCHING func_L00_0024A60C -- src/overlays/shared/menu_00249720.c
 * Best so far: BYTES 2/76 (97.4% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Returns (x >= 233.0f && x <= 235.0f && D_0013D4EB != 0); p0.c matches all instructions except the first bc1f.
 *   Retail's first bc1f branches backward (-8 words) into the previous function's (func_L00_0024A5C0) trailing jr 
 *   A cross-function branch has no C form here; would need the two functions compiled as one unit or an assembler/
 *   fz4 z08: first bc1f (.word 0x4500FFF8) branches backward to .L0024A604, before the function start (shared tail
 *   hq12 s04: stopped at once (QUEUE.md walls). Its first bc1f branches to .L0024A604, two instructions before the
 */
extern unsigned char D_0013D4EB NOT_SDA;

/* Tests x in [228, 235] and a menu flag. */
int func_L00_0024A60C(float a, float b, float x) {
    return x >= 233.0f && x <= 235.0f && D_0013D4EB != 0;
}
