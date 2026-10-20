/* func_L23_003C0C78 (380 bytes, level 23 only) -- NEAR MISS, not integrated.
   This C compiles to 380 bytes, retail's size. It belongs in the -G0 unit
   src/levels/23_damosel_orbit/L23_0035CE90.c in place of the marker (the unit declares the
   helpers with void * parameters further down; these declarations agree with them). check.py
   reports "instruction at +0x8 differs".
   Right: the size, both loops with retail's shape (the bc1f with the call argument in its slot,
   the nop at the inner loop entry, the k++ / reload / slt join, the index table reloaded through
   its bare lui/lw each pass), and the count: retail loads D_L23_001B1684 into $v1 for the
   `blez` and copies it into $s7 in the delay slot. That copy comes from reading the global in
   the loop test with the global declared `const` (the hoisted load is merged with the guard's);
   a local `count` loads it straight into the saved register (376 bytes).
   What differs:
   - the entry block, 27 words between +0x08 and +0x7C: retail copies arg0+0x10 with
     `lq v0,0(a1); sq v0,0(sp)` (address in a register at offset 0, a1 also being the next
     call's argument), with all register saves, `s5 = a0`, `fp = a1`, `a1 = s5 + 0x10` and the
     state load before the copy and nothing after it moved above it: a scheduling barrier, which
     is what the inline-asm quadword copy helpers give (docs/COMPILER-NOTES.md, "Quadword copy
     source shapes"). This cc1 folds the offset and spreads the block;
   - i and the object pointer trade $s2/$s3 (about 12 words): global allocation ranks i
     (16 refs over 47 insns) just above the object pointer (9 over 20), so i takes $s2. In
     func_L23_00418DC0, whose loops are the same, i spans 92 insns, ranks below, and the
     registers come out as retail's. Moving `i = 0` in the source, declaring k and the object
     pointer elsewhere, an if-guard instead of `continue`, and a separate index variable were
     measured; none changed it. Whether the entry-block barrier is what lengthens i's range in
     retail is not established.
   What fixed the earlier 376-byte attempt: the indexed inner loop (`obj + 0x10 + k * 0x10`)
   and the const global in the loop test. func_L23_003C0F48 takes an int on this branch, so the
   call passes i without sign extension.
*/

typedef int TI16 __attribute__((mode(TI)));

extern int func_419CA0(int);

extern char D_00189EA0[];

extern const int D_L23_001B1684 __attribute__((sda));

extern int *D_L23_001B36A8 __attribute__((sda));

extern int *D_L23_001FF300[];

extern void func_00282CC0(void *, void *, void *);

extern void func_00282CF0(void *, void *, void *);

extern void func_00282F20(void *, void *, float);

extern float func_00282E48(void *, void *);

extern void func_L23_003C0F48(char *arg0, int arg1);

/* picks the nearest entry of the objects the index table names, and stores its index */
void func_L23_003C0C78(char *arg0, int arg1) {
    struct {
        TI16 a;
        float b[4];
    } loc;
    char *src = arg0 + 0x10;
    char *state;
    float best;
    int i;

    loc.a = *(TI16 *)src;
    state = *(char **)(arg0 + 0x68);
    func_00282CF0(loc.b, src, D_00189EA0);
    best = 30000.0f;
    func_00282F20(loc.b, loc.b, 4.0f);
    func_00282CC0(&loc.a, &loc.a, loc.b);
    for (i = 0; i < D_L23_001B1684; i++) {
        int *obj;
        int k;

        if (arg1 != 0 && func_419CA0(i) == 0)
            continue;
        obj = D_L23_001FF300[D_L23_001B36A8[i]];
        for (k = 0; k < *obj; k++) {
            float d = func_00282E48((char *)obj + 0x10 + k * 0x10, &loc.a);

            if (d < best) {
                best = d;
                func_L23_003C0F48(arg0, i);
                *(short *)(state + 0x2B6) = k;
            }
        }
    }
}
