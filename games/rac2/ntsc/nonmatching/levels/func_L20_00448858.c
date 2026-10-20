/* near-miss: func_L20_00448858 (20_yeedil, 444 bytes, 111 words).
 *
 * The attempt below reaches retail's exact frame (0x40, four saved GPRs and one saved FPR),
 * its exact instruction count for everything but three words, and retail's shape for the
 * first float block: the `mtc1 $zero,$f0` of the 0.0f constant is materialised in the
 * prologue, the compare is `c.lt.s $f0,$f2` with a plain `bc1f`, the load of state[0x20]
 * sits in the branch's delay slot, and the second 0.0f comparison is REMATERIALISED
 * (`mtc1 $zero,$f1` right before it) instead of being kept in a register across the calls.
 * 15 of 111 words match; the rest is the register allocation and three block layouts.
 *
 * What differs:
 *
 * 1. The callee-saved GPR roles are rotated.  Retail: $s0 = the state pointer, $s1 = the
 *    argument, $s2 = the `arg0+0x10` / `D_L20_001C0430+0x10` pointer.  This cc1: $s0 = that
 *    pointer, $s1 = the state, $s2 = the argument.  Measured as insensitive to: the order
 *    of the declarations, the pointer being a named variable or the address written inline,
 *    the pointer being assigned at the top of the function (that costs a fifth saved
 *    register), the parameter being `unsigned char *`, and the state pointer being read
 *    through a `void *`.  The retail's order (most used first) is what this cc1 produces
 *    for every other spelling tried.
 *
 * 2. The float registers: retail keeps the loaded state[0x20] in $f0 (re-using the register
 *    of the dead 0.0f constant) and the 4.0f factor in $f1; this cc1 puts the load in $f1
 *    and the 4.0f in $f0.
 *
 * 3. The layout of the first `if`: retail has the multiply as the fall-through with no
 *    branch at all, this cc1 emits `b` over the else block.  Swapping the two arms of the
 *    last if/else in the function (so that the call is the fall-through, as in retail)
 *    flips this one but moves two other blocks: the whole function then starts four bytes
 *    earlier (8 of 111 words).
 *
 * 4. Three stores/loads are placed in a call's delay slot here and before the call in
 *    retail: the `swc1 $f0,0x8($s0)` of the first result, the `sq $v0,0x0($s2)` of the
 *    128-bit copy and the `daddu $a1,$s2,$zero` of the first call.
 *
 * Measured and not enough: the `-g`, `-mno-split-addresses`, `-fno-schedule-insns`,
 * `-fno-schedule-insns2`, `-fcse-skip-blocks`, `-fcse-follow-jumps`, `-fno-gcse`,
 * `-fedge-sm`, `-fedge-lm`, `-fexpensive-optimizations` and `-fno-expensive-optimizations`
 * per-file flags (all leave it at 13-15 of 111 words; `-fno-schedule-insns` and
 * `-fno-schedule-insns2` reorder the whole body).
 *
 * Compiler notes worth keeping (measured on this body):
 *
 * - The two 0.0f constants of this function are MERGED by this cc1 when the first
 *   comparison is written as `if (0.0f < f2)`, so the constant is live across four calls
 *   and lands in $f20 (a saved FPR).  Writing the comparison with an `if/else` that has an
 *   arm of its own (`if (z < f2) { f0 = a * 4.0f; } else { f0 = a; }`) keeps them apart:
 *   the constant then stays in the caller-saved $f0 and the second comparison is
 *   rematerialised.  With `if (0.0f < f2)` and no else arm there is no spelling that
 *   avoids the merge (tried: a user variable for the constant, `f2 > 0.0f`,
 *   `!(0.0f >= f2)`, a user variable for the second constant, `volatile`, the constant
 *   spelled as a double).
 *
 * - The comparison's operand can BE the variable: `float f0 = 0.0f; if (f0 < f2) {...}`
 *   keeps the 0.0f in $f0 without an else arm (the constant is a user variable, so it is
 *   not merged with the literal of the second comparison) and the second comparison is
 *   rematerialised - that is the retail's shape.  What it does not reach is the retail's
 *   BLOCK LAYOUT: with an else arm this cc1 emits `bc1f` and puts the multiply in the
 *   fall-through, but it also emits an extra `b` and a copy of the `else` arm; without an
 *   else arm it emits `bc1fl` (the likely form) with the else path in the fall-through and
 *   a duplicated `sub.s`.  The retail has one `bc1f`, one `sub.s` and the multiply in the
 *   fall-through, i.e. its load of state[0x20] sits in the branch's delay slot and its
 *   then-block uses that value - this cc1 does not move a load out of a block into the
 *   delay slot of a predecessor branch.
 *
 * - The closest spelling on the FLOAT registers is the conditional expression
 *   `*(float *)(state + 8) = f2 - (f0 < f2 ? *(float *)(state + 0x20) * 4.0f : f0);`
 *   with `float f0 = 0.0f;`: it puts the loaded state[0x20] in $f0, the 4.0f factor in
 *   $f1, emits `mul.s $f0,$f0,$f1` exactly as retail does, and materialises the second
 *   0.0f comparison afresh (`mtc1 $zero,$f1`) as retail does.  What is left is the
 *   `bc1fl` against retail's plain `bc1f`, the load staying in the then-block instead of
 *   the delay slot, and the GPR rotation.
 */
typedef int TI128 __attribute__((mode(TI)));

extern void func_00282CC0(void *, void *, void *);
extern float func_00283B98(float, float);
extern int func_00277258(void *, int, void *, float);
extern float func_00282DB0(void *, void *);
extern void func_00282F20(char *, char *, float);
extern float func_00282DF0(void *);
extern void func_00282D30(void *, void *, float);
extern int func_00282948(void *);
extern void func_0029F338(char *);

extern TI128 D_L20_001C0430;

void func_L20_00448858(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *p;
    TI128 v;
    float f;

    {
        float f2 = *(float *)(state + 8);
        float a = *(float *)(state + 0x20);
        float f0;
        float z = 0.0f;

        if (z < f2) {
            f0 = a * 4.0f;
        } else {
            f0 = a;
        }
        *(float *)(state + 8) = f2 - f0;
    }

    func_00282CC0(arg0 + 0x10, arg0 + 0x10, state);
    *(float *)(arg0 + 0xF0) =
        func_00283B98(*(float *)(arg0 + 0xF0), *(float *)(state + 0x10));
    *(float *)(arg0 + 0xF4) =
        func_00283B98(*(float *)(arg0 + 0xF4), *(float *)(state + 0x14));
    *(float *)(arg0 + 0xF8) =
        func_00283B98(*(float *)(arg0 + 0xF8), *(float *)(state + 0x18));

    if (func_00277258(arg0 + 0x10, 0, arg0, 1.5f) != 0) {
        p = (char *)&D_L20_001C0430;
        *(TI128 *)(arg0 + 0x10) = *(TI128 *)p;
        {
            float f1 = func_00282DB0(state, p + 0x10);

            if (f1 < 0.0f) {
                func_00282F20(p + 0x10, p + 0x10, 1.0f);
                f = func_00282DF0(state);
                f1 = func_00282DB0(state, p + 0x10);
                func_00282D30(&v, p + 0x10, -f1 * 1.25f);
                func_00282CC0(state, state, &v);
                func_00282F20(state, state, f * 0.5f);
            }
        }
    }

    *(int *)(state + 0x24) = 0;
    if ((unsigned char)arg0[0x23] >= 0x61) {
        arg0[0x23] = 0x60;
    }
    if (func_00282948(state + 0x24) != 0) {
        if ((unsigned char)arg0[0x23] < 5) {
            arg0[0x23] = arg0[0x23] - 4;
        } else {
            func_0029F338(arg0);
        }
    }
}
