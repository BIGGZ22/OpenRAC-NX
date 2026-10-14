/* NON_MATCHING func_L00_0024F170 -- src/overlays/shared/missionfunc_0024EEF0.c
 * Best so far: SIZE ours 80 / retail 84, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Selects mission status from two flags and an enable byte.
 *   Stopped at repeated lui wall: p2 repeats D_0013D5CA high address; base retention and return control also diffe
 *   A per-function compiler flag investigation would unblock this.
 */
extern char D_0013D6B8[];
extern unsigned char D_0013D5CA MACRO_ADDR;
// Selects mission status from flags.
int func_L00_0024F170(void) {
 char *p = D_0013D6B8;
 int r=0;
 if (*(int *)(p+0x14C) && !*(int *)(p+0x18C)) return 1;
 if (*(int *)(p+0x18C) && D_0013D5CA) {
  r=2;
  /* FIX (rv02): retail `beqz $3,.L0024F1BC` with $2=2 in its delay slot returns 2 when +0x16C is ZERO and falls to the return-0 when it is nonzero. */
  if (*(int *)(p+0x16C)) r=0;
 }
 return r;
}
