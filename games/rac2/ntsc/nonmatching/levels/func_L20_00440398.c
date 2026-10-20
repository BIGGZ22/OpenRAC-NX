/* Near miss from src/levels/20_yeedil/L20_0043ECB0.c: 196 bytes of retail, 180
   bytes of candidate (four instructions short).

   This unit has no $gp access, so the body belongs here (-G0), not in the -G8
   sibling.

   What matches: the whole walk.  The table lookup
   (`D_L20_001FBB20[index]`, the `& 0xFF` guard, the `D_L20_001B2FB8` range
   guard, the null guard), the shared exit that returns the counter, the
   `base + ((id & 0x7FFF) << 8)` address, the `!= 6` and `>= 0` tests with
   the `movz` counter update, the `rec == 0` guard, the `bgezl`/`beql` pair
   that ends the walk, and the `daddu $v0,$a3,$zero` return are the retail's
   instructions, in the retail's registers.

   What is missing (four instructions):
   1. the retail tests a second flag inside the loop that this candidate
      cannot produce:
          daddu $t0, $zero, $zero    (before the loop)
          bnez   $t0, .L00440454     (at the bottom of the loop)
          nop
      The tested value is always zero, so the source spelling has to be one
      this cc1 cannot fold away.  Measured folded (the branch disappears):
      `int`/`u32`/`u16`/`u8`/`long long`/`u64` locals set to 0, a pointer set
      to 0, `flag != 0`, `index - index`, `m - m`, `rec - rec`, `count -
      count`, `index & 0`, `index ^ index`, `(u32)index * 0`, a `static`
      local, and the same flag declared `volatile`.  The retail's own
      `daddu $t0,$zero,$zero` + `bnez $t0` pattern occurs nowhere else in
      the game, so it is a rare source construct, not a common one.
   2. because that flag needs `$t0`, the retail copies the masked index out
      of it: `daddu $a2, $t0, $zero` (the retail compares `$a2` with 0xFF
      inside the loop, this candidate compares `$t0` in place).
   3. the retail loads the object bank base with an absolute
      `lui $t1,%hi(D_1B2F9C) ; lw $t1,%lo(D_1B2F9C)($t1)`; this candidate
      emits the same `lw` in a delay slot, where the assembler has to keep
      it `$gp`-relative (one instruction), so the base ends up in `$a2`
      instead of `$t1`.

   Spellings tried for the loop: `do { } while (cond)`, `for (;;)` with
   `goto`, the counter initialised before and after the guards (the retail
   initialises it after the masked index and before the first branch, so
   `int count = 0;` sits between them), a single `if (a && b && c)` around
   the loop, and the counter as a `u32`.  The `count` initialisation must
   stay before the first guard branch (the retail's `daddu $a3,$zero,$zero`
   is at +0x20) and the guards must share the single exit, which needs the
   counter to be unknown to this cc1 on the guard paths.

   The `||` in the walk condition is the retail's: `*(s16 *)(rec - 1) >= 0`
   or `rec == (u16 *)D_L20_001FBB20[index]`. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;

extern char *D_L20_001FBB20[];
extern unsigned int D_L20_001B2F9C __attribute__((sda));
extern int D_L20_001B2FB8 __attribute__((sda));

int func_L20_00440398(int index) {
    u16 *rec = (u16 *)D_L20_001FBB20[index];
    int m = index & 0xFF;
    int count = 0;

    if (m == 0xFF) {
        goto end;
    }
    if (D_L20_001B2FB8 < index) {
        goto end;
    }
    if (rec == 0) {
        goto end;
    }
    {
        char *base = (char *)D_L20_001B2F9C;

        do {
            u16 word = *rec;
            char *e = base + ((word & 0x7FFF) << 8);

            if (e == 0) {
                goto end;
            }
            if (*(u8 *)(e + 0x20) != 6) {
                if (*(s8 *)(e + 0x20) >= 0) {
                    count++;
                }
            }
            if (m == 0xFF) {
                goto end;
            }
            rec++;
            if (rec == 0) {
                goto end;
            }
        } while (*(s16 *)(rec - 1) >= 0 || rec == (u16 *)D_L20_001FBB20[index]);
    }
end:
    return count;
}

/* ---- wave 3 (20_yeedil, 2026-10-20, worker 02): the always-zero guard, measured -----
 *
 * The walk is still the wave-2 candidate (180 bytes against the retail's 196: the
 * guard's three instructions and the copy of the masked index out of $t0 are missing).
 * What is new is what does NOT produce the guard:
 *
 *  - The assignment INSIDE the loop (`flag = 0;` at the top of the body, with
 *    `if (flag) goto end;` at the bottom) is hoisted out of the loop by the loop
 *    optimiser - and the branch is folded with it: both the assignment and the test
 *    disappear.  The hoisted copy would have been the retail's `daddu $t0,$zero,$zero`,
 *    but the test never survives the folding.
 *  - Twenty spellings of the flag were scanned in the loop's own context (int/u32/
 *    u16/s8/s16/s64/short/char/float/void*/volatile/static locals, `flag`/`flag != 0`/
 *    `flag >= 0`/`flag > 0`/`!(flag == 0)`/`!!flag`/`flag ? 1 : 0` as the test, the
 *    flag assigned inside the loop, the flag assigned to itself, the flag as a
 *    `static`): every one of them is folded, and none leaves a `daddu $t0,$zero,$zero`
 *    behind.  The scan's output has no `bnez $t0` either.
 *  - What DOES keep a test alive, and is the way out for a similar guard: a 64-bit
 *    copy of an unknown 32-bit value (`s64 flag = x;` with `x` unknown) keeps the
 *    comparison (`bne $4,$0`) because the front end cannot fold a 64-bit comparison
 *    it cannot see through - but then the value is not zero and the retail's
 *    `daddu $t0,$zero,$zero` has no source.
 *  - The `x - x`/`x ^ x` family (with the two operands in two variables, in a struct,
 *    in an array, or one a copy of the other) is folded by the front end as well.
 *
 *  Conclusion so far: the retail's `daddu $t0,$zero,$zero` + `bnez $t0` is a branch on
 *  a value the compiler has just materialised as zero, and no plain-C spelling of a
 *  zero-valued local survives it in this cc1.  The same pattern appears in
 *  func_L20_0039E0A0 and func_L20_0039DBC8 (other units) - it is a game-wide idiom,
 *  so a construct that produces it is worth a search of its own.
 */
