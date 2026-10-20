/* func_L23_00418DC0 (360 bytes, level 23 only) -- NEAR MISS, not integrated.
   This C compiles to 360 bytes, retail's size. Only the entry block differs: 28 words between
   +0x04 and +0x80 (the register saves, the 16-byte copy of arg0+0x10, the first call's
   arguments, the two float constants and i = 0 are interleaved in another order). From +0x84 to
   the end, including the loop registers (k in $s1, the object in $s2, i in $s3) and the nop at
   the inner loop entry, it matches. It belongs in the -G0 unit src/levels/23_damosel_orbit/L23_00416388.c; check.py
   reports "instruction at +0x8 differs".
   What fixed the earlier 356-byte attempt: the inner loop indexes the entries
   (`obj + 0x10 + k * 0x10`) instead of walking a pointer, which keeps sched1 from hoisting the
   pointer increment and the call arguments above the compare; and `char *slots` with
   `slots + (i << 2)` gives retail's `addu v0,s7,v0`.
   What remains is the entry block. Retail copies with `lq v0,0(a1); sq v0,0(sp)`, the address
   in a register at offset 0 (a1 = arg0 + 0x10, also the next call's argument), with every
   register save, `s5 = a0`, `fp = a1`, `a1 = s5 + 0x10` and the state load scheduled before the
   copy and nothing after it moved above it. That is a scheduling barrier, which is what the
   inline-asm quadword copy helpers give (docs/COMPILER-NOTES.md, "Quadword copy source
   shapes"); this cc1 folds the offset (`lq v0,0x10(s5)`) and spreads the block. Statement order
   of the copy and the state load was measured without effect.
   func_L23_00419088 is called unprototyped because the unit defines it with an s16 parameter
   (a prototype makes this caller sign-extend i). Measured on 20 October 2026: that definition
   still matches 1 of 1 with an s32 parameter, so whoever integrates this can prototype the call.
*/

typedef int s32;
typedef short s16;
typedef int TI128 __attribute__((mode(TI)));

extern void func_00282CF0(float *, const float *, const float *);

extern void func_00282F20(char *, char *, float);

extern void func_00282CC0(char *, char *, char *);

extern float func_00282E48(char *, char *);

extern char D_00189EA0[];

extern int *D_L23_001FF300[];

extern s32 func_L23_00419CA0(s32 arg0);

/* Picks the nearest entry of the object each measured slot names, and stores the index found. */
void func_L23_00418DC0(char *arg0, s32 arg1) {
    extern void func_L23_00419088();
    struct {
        TI128 a;
        float b[4];
    } loc;
    char *src = arg0 + 0x10;
    char *state;
    char *slots;
    float best;
    s32 i;

    loc.a = *(TI128 *)src;
    state = *(char **)(arg0 + 0x68);
    slots = state + 0x230;
    func_00282CF0(loc.b, (float *)src, (float *)D_00189EA0);
    best = 30000.0f;
    func_00282F20((char *)loc.b, (char *)loc.b, 4.0f);
    func_00282CC0((char *)&loc.a, (char *)&loc.a, (char *)loc.b);
    for (i = 0; i < 0x2F; i++) {
        int *obj;
        s32 k;

        if (arg1 != 0 && func_L23_00419CA0(i) == 0)
            continue;
        obj = D_L23_001FF300[*(int *)(slots + (i << 2))];
        for (k = 0; k < *obj; k++) {
            float d = func_00282E48((char *)obj + 0x10 + k * 0x10, (char *)&loc.a);

            if (d < best) {
                best = d;
                func_L23_00419088(arg0, i);
                *(s16 *)(state + 0x396) = k;
            }
        }
    }
}
