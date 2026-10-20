/* func_L23_00418BC8 (504 bytes, level 23 only) -- NEAR MISS, not integrated.
   Belongs in src/levels/23_damosel_orbit/L23_00416388-g8.c (it reads D_L23_001B16B8 through
   $gp, so the game compiler builds it with -G8).
   state: 496 bytes against retail's 504. WALL, not a wording problem: retail copies the
   argument vector into two locals with two separate quad copies through address registers,
       addiu $v1,$sp,0x40 ; lq $v0,0($s2) ; sq $v0,0($v1)
       addiu $a0,$sp,0x20 ; lq $v0,0($s2) ; sq $v0,0($a0)    (the second, separate load)
   and, in the loop, `lq $v0,0($s2) ; sq $v0,0($fp)` (fp = &v20). This cc1 folds the frame
   offsets (`sq $v0,0x40($sp)` and `sq $v0,0x20($sp)` from one `lq`): the two address
   `addiu`s and one load are missing. It is the wall already recorded for the L08 quad
   copies in docs/COMPILER-NOTES.md; the TI128 copy is the only spelling that reaches
   `lq`/`sq`, and an aggregate copy gives `ld`/`sd` pairs.
   The rest differs in register numbering: retail keeps &v40 in $s4 (copied from the
   $v1 of the first quad copy) and a second name for it in $s7, &v50 in $s3, &v10 in $s5,
   the found flag in $s6 and 0.0 in $f20; this C numbers the same values $s3..$s7
   differently and sets the flag in $s7.
   What is right in structure: the 70.0 early exit, the loop with both branches carrying
   their own `func_L23_00418650` call (retail duplicates the tail in the two arms) and one
   shared `func_002761F8` test (a non-zero result returns 0), the `d < 0.0f` exit that
   returns 1 (f20 holds the constant), and the frame (0xC0, six locals at sp+0x00..0x5F:
   an array of TI128 locals keeps that order, scalars are laid out in the order of first
   address use). `p40` is the second name for &v40 that gives retail's copy
   `daddu $s7,$s4,$zero` (the loop ends with `func_00282CC0(p40, p40, &v10)`). */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B16B8 __attribute__((sda));

extern float func_00282E48(float *, float *);
extern void func_00282CC0(float *, float *, float *);
extern void func_00282CF0(float *, float *, float *);
extern void func_00282F20(float *, float *, float);
extern void func_L23_00418650(char *, float);
extern int func_002761F8(char *, char *, int, int, int);

int func_L23_00418BC8(char *obj, TI128 *pa, TI128 *pb) {
    char *state = *(char **)(obj + 0x68);
    TI128 loc[6];
    TI128 *p40;
    TI128 *cur;
    int found;

    if (func_00282E48((float *)pa, (float *)pb) > 70.0f)
        return 0;
    loc[0] = *pb;
    func_L23_00418650((char *)&loc[0], *(float *)(state + 0x54) + *(float *)(state + 0x50));
    func_00282CF0((float *)&loc[1], (float *)&loc[0], (float *)pa);
    func_00282F20((float *)&loc[1], (float *)&loc[1], D_L23_001B16B8);
    loc[4] = *pa;
    loc[2] = *pa;
    func_L23_00418650((char *)&loc[2], *(float *)(state + 0x54) + *(float *)(state + 0x50));
    p40 = &loc[4];
    found = 0;
    do {
        float d = func_00282E48((float *)&loc[4], (float *)&loc[0]);

        if (d < D_L23_001B16B8) {
            if (d < 0.0f)
                return 1;
            func_00282F20((float *)&loc[5], (float *)&loc[1], d);
            found = 1;
            func_00282CC0((float *)&loc[3], (float *)&loc[4], (float *)&loc[5]);
            cur = &loc[3];
            func_L23_00418650((char *)&loc[3], *(float *)(state + 0x54) + *(float *)(state + 0x50));
        } else {
            func_00282CC0((float *)&loc[3], (float *)&loc[4], (float *)&loc[1]);
            cur = &loc[3];
            func_L23_00418650((char *)&loc[3], *(float *)(state + 0x54) + *(float *)(state + 0x50));
        }
        if (func_002761F8((char *)&loc[2], (char *)cur, 0x12, 0, 0) != 0)
            return 0;
        loc[2] = *cur;
        func_00282CC0((float *)p40, (float *)p40, (float *)&loc[1]);
    } while (found == 0);
    return 1;
}
