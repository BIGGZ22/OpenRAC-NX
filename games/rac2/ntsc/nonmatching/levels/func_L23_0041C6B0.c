/* func_L23_0041C6B0 (292 bytes, 1 place: 23:0041C6B0) -- NEAR MISS, not integrated.
   This C compiles to 292 bytes, retail's size, and two words differ: in the if-body this cc1
   issues the quadword load first, retail issues the move first.
     ours:   lq v0,0(s0) / mov.s f20,f0 / sq v0,0(s5)
     retail: mov.s f20,f0 / lq v0,0(s0) / sq v0,0(s5)
   It belongs in the -G8 sibling src/levels/23_damosel_orbit/L23_00416388-g8.c (D_L23_001B1484 is
   reached through $gp; the sibling already declares it). check.py there reports "instruction at
   +0xc4 differs".
   What fixed the earlier 288-byte attempt (bc1f with j++ in its slot):
   - the inner loop indexes the entries (`list + 0x10 + j * 0x10` through one local `e`) instead
     of walking a pointer: the strength-reduced pointer is then set after the `*list > 0` guard,
     j = 0 lands in the blez slot, j++ stays at the join, and the compare becomes retail's bc1fl
     with the count reload in its slot;
   - `char *slots` with `slots + (i << 2)` gives retail's `addu v0,s6,v0` (an `int *` indexed
     with [i], or `i * 4`, gives the index first).
   What remains: sched1 always puts the lq before the independent mov.s (the lq has the longer
   path). Retail keeps lq/sq together after the move, which is what a single unsplittable copy
   insn (the inline-asm quadword helpers of docs/COMPILER-NOTES.md) does. Statement order, a
   temporary for the vector, and both compare polarities were measured; none moved it.
*/

typedef int TI128 __attribute__((mode(TI)));

extern char *D_L23_001B1484 __attribute__((sda));

extern float func_00282E48(char *, char *);

extern int *D_L23_001FF300[];

extern int func_L23_00419CA0(int);

/* copies the entry nearest to arg1 among the objects the measured slots name into arg0 */
void func_L23_0041C6B0(char *arg0, char *arg1, int arg2) {
    char *state = *(char **)(D_L23_001B1484 + 0x68);
    char *slots = state + 0x230;
    float best = 30000.0f;
    int i;

    for (i = 0; i < 0x2F; i++) {
        if (arg2 == 0 || func_L23_00419CA0(i) != 0) {
            int *list = D_L23_001FF300[*(int *)(slots + (i << 2))];
            int j;

            for (j = 0; j < *list; j++) {
                char *e = (char *)list + 0x10 + j * 0x10;
                float d = func_00282E48(e, arg1);

                if (d < best) {
                    best = d;
                    *(TI128 *)arg0 = *(TI128 *)e;
                }
            }
        }
    }
}
