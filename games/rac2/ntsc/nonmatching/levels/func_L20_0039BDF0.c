/* Near miss from src/levels/20_yeedil/L20_002DEBE0.c.

   The body has the retail's frame, the same calls in the same order, the same
   register assignment ($s3 <- the fourth argument, $s4 <- the third), the
   32-bit address arithmetic and the retail's return value (a pointer, so the
   epilogue's `daddu $v0,$s0,$zero`), but three things differ:

     - the two 16-byte copies: retail materialises the pointer first
       (`addiu $a0,$s0,0x10` / `sq $v0,0x0($a0)`, and `addiu $a1,$s0,0xF0` /
       `sq $zero,0x0($a1)`), this compiler folds the constant offset into the
       `sq` (`sq $v0,0x10($s0)`, `sq $zero,0xF0($s0)`);
     - the float constants (2.0f, 0.5f) are materialised just before their use
       in the retail, this compiler hoists them in front of the 16-byte copy;
     - the second call's delay slot holds the store to arg0+0x98, the retail
       has `addiu $a1,$s0,0xF0` there (and recomputes p+0xF0 for the call,
       because the call clobbers $a1).

   The copy is the wall, and it is a compiler-level one, not a source shape:
   the fold happens in the CSE pass, in `find_best_addr` (gcc/cse.c), which
   replaces a memory operand's address register with the equivalent expression
   recorded for it when that expression is not more expensive to address.  The
   cost comes from `mips_address_cost` (gcc/config/mips/mips.c): a plus of a
   register and a *small* constant (SMALL_INT: -0x8000..0x7FFF) costs 1, the
   same as a plain register, and the tie is broken in favour of the plus, so
   `p + 0x10` is folded into the `sq`.  The retail's compiler did not fold it.

   Tried: the address as a `char *`, an `int` and an `unsigned int`; the
   address computed inside and outside the `if (p)` block; the address used
   once and twice (for the copy and as the first argument of the call); the
   copy through a `volatile` TI128 pointer (the fold is not a CSE of the
   value, so volatile does not stop it); the address as a 64-bit value (the
   fold is then impossible - the memory operand's address mode is 32-bit - but
   the address computation becomes a 64-bit `daddu`/`daddiu` instead of the
   retail's 32-bit `addiu`, and the sign/zero extension of the 32-bit address
   costs two more instructions); the offset written so that it is not a small
   integer (`p - 0xFFF0`): the fold is then avoided, but the constant no
   longer fits in a signed 16-bit immediate, so the compiler puts it in a
   register (`addu $a0,$a0,$2`).

   So: a TI copy through a register needs an address that is not a register
   plus a small constant, and the retail's addresses here are exactly that.

   ------------------------------------------------------------------
   2026-10-20, worker 16 of the level-20 wave.  The attempt below is unchanged
   (208 of 212 bytes, "tools/check.py func_L20_0039BDF0: 208 bytes, expected
   212").  What this wave measured:

   * The retail's shape is not a rare accident: it materialises a 128-bit
     address in a register whenever the address is used more than once
     (`addiu $a0,$s0,0x10` + `sq $v0,0x0($a0)`, the address also being the
     first argument of the call) and folds the offset into the instruction when
     it is used once.  Level 20 has 6840 `lq`/`sq` accesses, 1349 of them
     folded into the instruction and the rest through a register - so both
     shapes are the compiler's, and the source decides which one comes out.

   * Where the fold happens, exactly: the front end expands the address of a
     `*(TI128 *)(p + 0x10)` into a pseudo register (`expand_expr` with
     EXPAND_SUM -> binop), the memory operand is `mem:TI (reg $pseudo)`, and
     the CSE's `find_best_addr` (gcc/cse.c) then replaces that register by the
     equivalent expression of its equivalence class - the plus - because
     `mips_address_cost` (gcc/config/mips/mips.c) gives "register plus a small
     constant" the cost 1, the same as a plain register, and the tie is broken
     for the expression with the higher `rtx_cost`.  The plus is in the class
     because `cse_insn` inserts the source of every `set` into the class of
     its destination.  The reload pass would have materialised the pseudo into
     a hard register (that is where the retail's `addiu $a0,$s0,0x10` comes
     from: it is the reload of the address pseudo, and the store through it is
     `sq $v0,0x0($a0)`), but the CSE has removed the pseudo before the reload
     pass sees it.  The SDK's ee gcc (the other profile, the unpatched 2.9
     tree) folds exactly the same way, so this is not a regression of the
     0047/0048 path-B patches.

   * The escapes measured this wave, all of them fold:
       - the address in a local pointer variable used twice (the store and the
         call argument): `sq $v0,0x10($s0)`, the address is materialised only
         for the call argument;
       - the address computed *before* the `if (p)`, so that it belongs to the
         previous basic block: the CSE joins the blocks (it follows the
         conditional jump) and the equivalence class survives the branch, so
         the store is folded as well; the same with `-fno-cse-follow-jumps`,
         `-fno-cse-skip-blocks` and `-fno-gcse`;
       - a call between the address computation and the store: the class is
         rebuilt and the fold still happens;
       - the address as a `TI128 *` arithmetic (`((TI128 *)p) + 1`, which is
         p+16): folded;
       - `-fno-expensive-optimizations`, `-fforce-addr`: no change of shape.

   * The one escape that reaches the retail's shape is the volatile pointer
     object of the wave's note (`TI128 *volatile d`): the store becomes
     `sq $v0,0x0($reg)`, but the pointer has to go through the stack, so the
     body is `sw`/`lw` around it and the whole function is 236 bytes instead
     of 212 (24 bytes over).  No spelling keeps the zero-displacement `sq`
     without the stack round trip: the address has to reach the store either
     as a plus (folded) or as a value loaded from memory (the stack).

   * The two other differences follow from the copy: with the address in a
     register the schedule changes, the float constants (2.0f, 0.5f) are
     materialised just before their use as in the retail, and the second
     call's delay slot holds `addiu $a1,$s0,0xF0` instead of the store to
     arg0+0x98.  So the copy is the one thing to solve.

   * What would unblock it: a source shape in which the address pseudo's
     equivalence class holds no plus at the store, i.e. an address that is
     loaded from memory (the stack, which costs the round trip) or one that
     the CSE cannot see as a plus.  A compiler that does not fold a TImode
     address into the instruction would reproduce the retail's shape; this
     compiler's `GO_IF_LEGITIMATE_ADDRESS` accepts "register plus a constant"
     for TImode while the offset is in [-32768, 32752], and an offset outside
      that range does make the reload materialise the address
      (`addu $a0,$s0,32753` + `sq $v0,0x0($a0)`, the retail's shape but with the
      wrong immediate). */

   ------------------------------------------------------------------
   2026-10-20, worker 15 of the level-20 wave (wave 3 follow-up).

   The attempt below is unchanged: "tools/check.py func_L20_0039BDF0:
   208 bytes, expected 212".  What this wave measured, and where the
   wall really is:

   * The fold happens in two passes, and the second one decides the shape:

     - the front end builds the address of the 16-byte store as the pseudo
       register that holds "p + 0x10";
     - the CSE (gcc/cse.c, find_best_addr) replaces that pseudo by the class
       member "plus(reg, 0x10)", because mips_address_cost
       (gcc/config/mips/mips.c) gives "register + a small constant" the cost
       1, the same as a plain register, and the tie is broken on the rtx
       cost, which favours the plus (notreg_cost is rtx_cost*2, so the plus
       costs 10 and the pseudo 1);
     - the change is validated by apply_change_group (gcc/recog.c), which
       asks memory_address_p, i.e. GO_IF_LEGITIMATE_ADDRESS
       (gcc/config/mips/mips.h): for a TImode access a plus is legitimate
       while INTVAL >= -32768 and INTVAL + GET_MODE_SIZE(TImode) - 1 <=
       32767, that is while the offset is in [-32768, 32752];
     - the reload pass then keeps that offset in the instruction.

     So the address stays in a register exactly when the offset is OUT of
     that range: "*(TI128 *)((char *)arg0 + 32753)" gives "addiu $v0,$v0,
     0x7FF1" + "sq $v3,0($v0)" - the retail's shape with the wrong
     immediate.  The immediate IS the offset, so no base bias can land the
     retail's 0x10 there.

   * The escapes, measured in this function.  All of them keep the address
     in a register; none of them keeps the retail's arithmetic:

     - the volatile pointer object: 216 bytes (4 over) - "sw $a0,0($sp)" +
       "lw $v1,0($sp)" + "sq $v0,0($v1)": the retail's shape, the stack
       round trip as the price;
     - an address that is not a plus of a register and a constant (a plus
       with a variable offset, or a pointer carried by a loop and
       incremented at the end of the body): the address stays in a
       register, but the compiler computes it with "addu $a0,$s0,$sX"
       (register + register) instead of the retail's "addiu" (register +
       constant);
     - a 64-bit (DI mode) address: "daddu $2,$4,16" + "dsll $2,$2,32" +
       "dsra $2,$2,32" + "sq $3,0($2)" - the address in a register, the
       arithmetic 64-bit and two instructions wider;
     - a pointer given DIFFERENT VALUES in two branches (the wave's second
       new escape): the value is unknown at the join, the CSE cannot fold,
       and the address stays in a register - but the compiler emits the
       second address computation and a "movn" to select between the two,
       and the body grows to 220 bytes.  With the SAME value in both
       branches the compiler collapses them and the fold comes back.

   * What is left is one instruction: the retail materialises the address
     of the SECOND copy ("addiu $a1,$s0,0xF0" + "sq $zero,0($a1)") and
     recomputes it for the call in the delay slot, while this compiler
     folds it into the store ("sq $zero,0xF0($s0)") and computes it once
     for the call.  That is the whole 4 bytes.

   * What the retail does, and where the two compilers differ.  The retail
     keeps the address of a 128-bit access in a register when the address
     expression is used more than once: here the destination address p+0x10
     is the store's address AND the first argument of the call at 0x0039BE64,
     and the retail computes it once ("addiu $a0,$s0,0x10") and keeps it in
     $a0 across to the call.  The boot's func_00270598 does the same for
     three copies (both the source and the destination of each copy go
     through an "addiu" and a zero-displacement "lq"/"sq").  When the
     address is used once the retail folds it, exactly like this compiler:
     f_00302D30 has "sq $v0,0x10($a0)" and the matched func_L08_003391C0 has
     "lq $v1,0x10($a0)".  So the difference between the two compilers is
     the multi-use case: this one folds the address into the store and
     computes it again for the call, the retail keeps one register for both.

   * That makes the wall compiler-side, and it is a small one: the CSE must
     not substitute a plus into a TImode address (or the reload must keep
     the address in a register when it is used more than once).  No C
     spelling avoids the fold: the address has to be the store's address
     AND the call's argument, and every escape that keeps the address in a
     register changes the instruction sequence (measured above). */

typedef int TI128 __attribute__((mode(TI)));

extern void *func_0029EFD0(int);

extern float func_002A89A8(void *, int, float);

extern void func_002833F0(void *, void *);

char *func_L20_0039BDF0(void *arg0, void *arg1, int arg2, int arg3)
{
    char *p;
    int addr;
    int addr2;
    int b;

    p = (char *)func_0029EFD0(0x12C2);
    if (p) {
        *(int *)(p + 0xB8) = (int)arg0;
        *(unsigned char *)(p + 0x30) = 0xFF;
        addr = (int)p + 0x10;
        *(TI128 *)addr = *(TI128 *)arg1;
        *(int *)(arg0 + 0x98) = 0;
        *(float *)(p + 0x18) = *(float *)(p + 0x18) + 2.0f;
        *(float *)(p + 0x18) = func_002A89A8((void *)addr, 0, 0.5f);
        addr2 = (int)p + 0xF0;
        *(TI128 *)addr2 = 0;
        b = *(int *)(p + 0xB8);
        *(int *)(b + 0x98) = *(int *)(*(int *)(b + 0x24) + 0x10);
        func_002833F0((void *)((int)p + 0xC0), (void *)addr2);
        b = *(int *)(p + 0x68);
        *(int *)(b + 0x4) = arg2;
        *(int *)(b + 0x8) = arg3;
    }
    return p;
}
