/* Near miss from src/levels/20_yeedil/L20_0039C448.c (worker 07, level-20 wave of
   2026-10-20): 300 bytes in the retail, 268 bytes here.  The frame (0x30), the
   three saved registers (s0, s1, ra), the table lookup, the object search, the
   16-byte zero store (`sq $zero,0($sp)`), the two float calls and the branch
   structure are all there.  What is left:

     1. The register allocation.  The retail puts the found object in $s0 and the
        copy of the second argument in $s1; this cc1 puts the copy in $s0 and the
        found object in $s1 (and the same swap for $a3/$v1).  The instruction
        sequence of the prologue is otherwise the retail's, word for word.

     2. The test of a variable that is provably zero.  The retail keeps
        `daddu $t1,$zero,$zero` and `bnez $t1`; this cc1 folds the whole thing
        away.  Making the variable uninitialised and assigning it zero inside the
        search block does not help: the assignment dominates the test, so the
        constant is propagated and the branch goes away.  The same problem is
        reported for func_L20_0039E0A0.

     3. The conditional assignment after the last call.  The retail has
        `movz $s0,$zero,$v0` (if the call returned zero, clear the found object)
        and then returns it; this cc1 proves that the value returned is the
        call's result and drops the conditional move.  A source shape that keeps
        the move was not found (the ternary and the `if` form both fold).

     4. The branch-likely delay slots in the list walk.  The retail has
        `bgezl $v0,loop / lhu $v0,0($a2)` and `beql $a2,$v0,loop / lhu $v0,0($a2)`;
        this cc1 emits the plain `bgez` with a nop and keeps the loads apart.

     5. The global accesses.  The retail loads D_L20_001B2FB8 and
        D_L20_001B2F9C as `lui $v0,%hi(S) ; lw $v0,%lo(S)($v0)` - one register
        for both halves.  With `__attribute__((sda))` this cc1 emits the $gp
        relative form for one of the two and the lui/lw form for the other; which
        of the two comes out in which form depends on the declaration, so the
        pair needs the double-name pattern to be pinned down.

   This body belongs in the `-G8` unit?  No: it reaches no global through $gp.
*/

typedef int TI128_w07 __attribute__((mode(TI)));

extern int D_L20_001BB20[];

extern int D_L20_001B2FB8_b __asm__("D_L20_001B2FB8") __attribute__((sda));

extern float D_L20_0012E20[];

extern int func_L20_0039DCF8(char *, char *, void *);

extern float func_00283248(float, float);

void func_L20_0039DBC8(s32 a0, s32 a1, char *arg2) {
    char *s0 = 0;
    char *s1 = arg2;
    u32 a3 = a1 & 0xFF;
    char *p = (char *)D_L20_001BB20[a1];
    int t1;
    u16 w;
    char *q;
    char *base;
    s32 v0;

    if (a3 != 0xFF) {
        if (D_L20_001B2FB8_b < a1) {
            goto after;
        }
        if (p == 0) {
            goto after;
        }
        base = D_L20_001B2F9C;
        t1 = 0;
        w = *(u16 *)p;
        q = base + ((w & 0x7FFF) << 8);
        if (q == 0) {
            goto after;
        }
        if (*(u8 *)(q + 0x20) != 6) {
            goto check;
        }
        s0 = q;
        *(s32 *)(s0 + 0xB8) = a0;
    }
check:
    if (a3 == 0xFF) {
        goto after;
    }
    p += 2;
    if (t1 != 0) {
        goto after;
    }
    while (p != 0) {
        if (*(s16 *)(p - 2) >= 0) {
            continue;
        }
        v0 = *(u16 *)p;
        if (p == (char *)D_L20_001BB20[a1]) {
            continue;
        }
        v0 = *(u16 *)p;
        break;
    }
after:
    if (s0 != 0) {
        TI128_w07 buf = 0;
        f32 f1 = *(f32 *)s1;
        f32 f0 = *(f32 *)(s1 + 4);
        *(f32 *)((char *)&buf + 8) = func_00283248(
            *(f32 *)((char *)D_L20_0012E20 + 0x80) - f1,
            *(f32 *)((char *)D_L20_0012E20 + 0x84) - f0);
        if (func_L20_0039DCF8(s0, s1, &buf) == 0) {
            s0 = 0;
        }
    }
    return s0;
}

/* ---- level-20 wave 3, worker 05 (2026-10-20): point 2 (the zero guard) is SOLVED ----
 *
 * The construct that keeps a test on a value the compiler proves is zero is
 *
 *     s32 flag = arg1 & ~arg1;
 *
 * The front end does not fold `x & ~x` (gcc/fold-const.c's fold() has no case for
 * BIT_AND with a non-constant second operand) and neither does the CSE; only
 * `combine` folds it, and combine runs AFTER gcse (toplev.c: cse -> gcse -> loop ->
 * cse2 -> flow -> combine_instructions).  gcse's cprop is the pass that would
 * otherwise propagate the constant 0 into the search loop and delete the branch, so
 * with `x & ~x` the zero is materialised in the register (`daddu $t1,$zero,$zero`)
 * and the retail's `bnez $t1` survives.  Measured on func_L20_0039E0A0: with
 * `flag = 0` the guard and its zeroing disappear (176 bytes against retail's 184);
 * with `flag = arg0 & ~arg0` both are there and every branch matches.
 *
 * The other three functions of the family need the same spelling
 * (func_L20_0039E0A0, func_L20_00440398, func_L20_0044AC38).
 *
 * What is still open here is the rest of the note above (the register allocation, the
 * conditional move after the last call, the branch-likely delay slots and the two
 * global accesses) plus the 128-bit copy wall: the retail materialises the copy
 * source with `addiu $a2,$a1,0x60` and copies with a zero offset, and every
 * spelling we tried folds the offset back into the lq (see
 * nonmatching/levels/func_L20_0043D470.c for the measurements). */
