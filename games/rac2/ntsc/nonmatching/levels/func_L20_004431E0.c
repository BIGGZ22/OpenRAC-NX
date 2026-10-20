/* Near miss from src/levels/20_yeedil/L20_0043ECB0-g8.c: 324 bytes of retail,
   324 bytes of candidate (the same size, the instructions differ from +0xc).

   The body reaches a global through $gp (the limit at 0x1AEFF0 + 0x3FC8), so it
   belongs in the -G8 sibling.

   The shape is the retail's: the table lookup `D_L20_001FBB20[state->0x218]`,
   the `& 0xFF` guard, the `D_L20_001B2FB8` range guard, the null guard, the
   `*(s16 *)(rec - 1) >= 0` and `rec == D_L20_001FBB20[idx]` pair, the
   `base + ((id & 0x7FFF) << 8)` address, the call to func_003BA3F8 with the
   `movn` that keeps the entry, the 16-byte copy of the second argument into
   the entry, the three calls and the 0.5f adjustment of the float at +8 of the
   copy.

   What differs is the register allocation and the order of the first block:
   the retail computes the table base in `$v0` (`lui $v0` + `addiu $a0,$v0`), the
   state in `$a2` and the index in `$v1`, and copies the table base into `$a3`
   in the delay slot of the jump to the guard block; the candidate computes the
   table base in `$v1` (with the `addiu` in the same register) before the state
   is loaded, and keeps the index in `$v0`/`$a3`.

   Measured (2026-10-20): the guard block has to be a separate block reached
   by a jump, with the body block laid out before it - a `goto` into a block
   gives the retail's order where an `if` does not.  The index must be read
   twice, once in the first block and once in the guard block, and this cc1
   folds the two reads into one unless the guard's read is `volatile` (the
   retail reads `state->0x218` twice and the table entry twice as well).

typedef unsigned short u16;
typedef signed short s16;

extern char *D_L20_001FBB20[];

extern char *D_L20_001B2F9C;

extern int D_L20_001B2FB8 __attribute__((sda));

extern int func_003BA3F8(char *);

extern void func_003BA368(char *);

extern void func_002FFA70(char *, char *, char *, int, int);

extern void func_0039BDF0(char *, char *, int, int, float);

void func_L20_004431E0(char *arg0, TI128 *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    u16 *rec = (u16 *)D_L20_001FBB20[*(int *)(state + 0x218)];
    char *found = 0;
    TI128 buf;
    int idx;

    goto check;
body:
    {
        char *e = (char *)D_L20_001B2F9C + ((*rec & 0x7FFF) << 8);

        if (e != 0) {
            if (func_003BA3F8(e) != 0) {
                found = e;
            }
        }
    }
    goto end;
check:
    idx = *(volatile int *)(state + 0x218);
    if ((idx & 0xFF) == 0xFF) {
        goto end;
    }
    if (D_L20_001B2FB8 < idx) {
        goto end;
    }
    if (rec == 0) {
        goto end;
    }
    if (*(s16 *)(rec - 1) >= 0 || rec == (u16 *)D_L20_001FBB20[idx]) {
        goto body;
    }
end:
    if (found != 0) {
        char *p = found + 0x10;
        char *s2;

        *(TI128 *)p = *arg1;
        func_003BA368(found);
        func_002FFA70(found, p, found + 0xF0, 0, 0);
        s2 = *(char **)(found + 0x68);
        *(int *)(s2 + 0xAC) |= 8;
        buf = *(TI128 *)p;
        *(float *)((char *)&buf + 8) -= 0.5f;
        func_0039BDF0(found, (char *)&buf, 0x168, 0, *(float *)((char *)&buf + 8));
    }
    return found;
}

/* ---- wave 3 (20_yeedil, 2026-10-20, worker 02): the first block, re-measured -------
 *
 * The candidate is still the retail's exact 324 bytes with the instructions differing
 * from +0xc, and the difference is the register allocation and the ORDER of the first
 * block only.  What was measured:
 *
 *  - The retail's first block is: `lui $v0,%hi(D_1FBB20)` (the %hi in $v0), the state
 *    in $a2, the index in $v1, `addiu $a0,$v0,%lo(D_1FBB20)` (the table's base in $a0),
 *    the record in $a1, then `b .L0044324C` with `daddu $a3,$v0,$zero` (the %hi
 *    copied) in the delay slot.  This compiler materialises the whole address in one
 *    register (`lui $v1,%hi; addiu $v1,$v1,%lo`) early in the prologue and copies the
 *    %hi into $t0 there, and it folds the `& 0xFF` test into the first block (the
 *    retail keeps it in the guard block the jump goes to).
 *  - The retail's split form (`%hi` in one register, the address in another, and the
 *    %hi copied for the second use) is the "split 64-bit la" wall of
 *    docs/COMPILER-NOTES.md; `-mno-split-addresses` gives the opposite shape (the
 *    address in one register), so no per-file flag reaches the retail's form here.
 *  - The index has to be read twice (once in the first block, once in the guard block);
 *    the guard's read must be `volatile` or this cc1 folds the two reads into one.
 *    That is in the candidate.
 *  - Measured and not enough: the table's element type (`u32 *` and `char *`), the
 *    index's type (`int`/`u32`), the volatile read as `volatile int` vs
 *    `volatile u32`, and the `-mno-split-addresses` profile (which changes the whole
 *    first block).
 */
