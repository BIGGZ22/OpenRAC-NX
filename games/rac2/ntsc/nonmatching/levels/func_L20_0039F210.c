#include "include_asm.h"

typedef unsigned int u32;
typedef int TI128 __attribute__((mode(TI)));

/* Near miss from src/levels/20_yeedil/L20_0039C448.c: 32 bytes; 28-byte candidate; 0/1.
    (Replaces an earlier draft from commit adcbd51, which used the inline-assembly `qcopy`
    helper that AGENTS.md's "Plain C" rule forbids; that draft is in that commit.)
    The only difference left is the 128-bit copy: retail materialises the destination
    pointer first (`addiu $v1,$a0,0x10; lq $v0,0($a1); sq $v0,0($v1)`) while this cc1 folds
    the constant offset into the store (`sq $v0,0x10($a0)`).  Measured again in this wave with
    about twenty further spellings, all of which fold the offset: a TI128 pointer variable, an
    array index, a struct member, a union, `volatile`, `aligned(1)`/`aligned(4)` TI types, an
    integer cast (signed and unsigned), a `short` truncation (which does keep the address in a
    register, but adds `sll`/`sra` and costs three instructions), a `unsigned long long` address
    (which keeps the address in a register, but adds `daddu` plus `dsll`/`dsra`), a pointer
    loaded from memory, a pointer used twice, two C names for the same pointer, and a pointer
    kept across a call.      The rule that holds: this cc1 folds a constant offset into the
    `lq`/`sq` whenever the address is a `plus` of a register; the address only stays in a
    register when it comes from something that is not a `plus` (a load, a call result) or when a
    mode conversion forces it (which then costs extra instructions).
    Same wall as the other TI copies in this level.

    Wave 20 (this worker) measured the same wall once more and adds what the retail does
    with the same shape: the retail folds the constant offset into the `lq`/`sq` in
    func_L08_003391C0 (`lq $v1,0x10($a0)`, matched), in func_L20_0043F510
    (`lq $v0,0x10($s0)`, matched) and in func_L20_00302D50 (all six accesses folded), and it
    materialises it in this function (`addiu $v1,$a0,0x10; lq $v0,0($a1); sq $v0,0($v1)`),
    in func_L20_0039E158, in func_L20_0039DCF8 (both stores) and in func_L20_004395B0 (both
    loads and one store).  So the retail has both behaviours and this cc1 only has the
    folding one; about thirty spellings were measured, in this function and in the other TI
    copies of this level, and all of them fold: a TI128 pointer variable (one and two of
    them), a struct member, an array index, a union, `volatile`, `aligned(1)`/`aligned(4)`
    TI types, an integer cast (signed and unsigned), a `short` truncation (which does keep
    the address in a register, but adds `sll`/`sra` and costs three instructions), a 64-bit
    address (which keeps the address in a register, but adds `daddu` plus `dsll`/`dsra`), a
    pointer loaded from memory, a pointer used twice, two C names for the same pointer, a
    pointer kept across a call and a `__builtin_memcpy` of 16 bytes (which comes out as
    `ldl`/`ldr`/`sdl`/`sdr`).
    The per-file flags do not help: `-g`, `-mno-split-addresses`, `-fno-schedule-insns` and
    `-fno-schedule-insns2` leave the fold exactly as it is.

    Wave 21 (this worker) measured the wall once more and adds three things the next
    worker can use:

    1. THE ASSEMBLER TURNS `addu rd,rs,imm` INTO `addiu rd,rs,imm`.  cc1 emits the
       materialised 128-bit address as a 64-bit `addu rd,rs,imm`, and the game's assembler
       (gas, tc-mips.c) rewrites it into the 32-bit `addiu` when the immediate fits in 16
       bits - so the retail's `addiu $v1,$a0,0x10` and this cc1's `addu $v1,$a0,16` are the
       same four bytes.  The same holds for `move rd,rs`, which becomes `daddu rd,rs,$zero`.
       Two consequences: (a) a 32-bit `addiu` in the retail is NOT evidence that the source
       did 32-bit arithmetic, and (b) a candidate that differs from the retail only in
       `addu`/`addiu` and `move`/`daddu` is NOT a difference - compare the objects, not
       the .s text (tools/check.py does; the listings do not).
    2. THE OUT-OF-RANGE OFFSET ESCAPE REALLY MATERIALISES THE ADDRESS, in the retail's
       instruction form.  `*(TI128 *)((char *)arg0 + 32753) = v;` compiles to
       `addu $v0,$v0,32753` -> `addiu $v0,$v0,0x7FF1` and `sq $v3,0($v0)`: the address stays
       in a register and the `sq` gets a ZERO displacement, exactly the retail's shape,
       because the TImode displacement range is [-32768, 32752] (measured: 0x7FF0 still
       folds, 0x7FF1 does not).  The immediate is wrong (0x7FF1 instead of the retail's
       0x10), and no bias of the base can fix it: the immediate in the instruction IS the
       offset the compiler used.  So the escape cannot land a body whose retail offset is
       inside the range, but it proves that the reload pass is what materialises the
       address - the retail's shape cannot come from the reload with an in-range offset,
       so it comes from the game's inline-assembly `qcopy` macro, whose operand must live
       in a register.
    3. The `volatile`-pointer escape (a `volatile` pointer OBJECT) reaches the zero
       displacement too, but it costs the stack round trip: `subu $sp,$sp,16 ; addu
       $v1,$a0,16 ; sw $v1,0($sp) ; lq $v0,0($a1) ; lw $v1,0($sp) ; sq $v0,0($v1)` - four
       instructions and sixteen bytes more than the retail's single `addiu`, so it can only
       land a body that is several bytes short. */
void func_L20_0039F210(char *arg0, char *arg1, int arg2) {
    char *dst = arg0 + 0x10;
    char *state;

    *(TI128 *)dst = *(TI128 *)arg1;
    state = *(char **)(arg0 + 0x68);
    *(int *)state = 5;
    *(int *)(state + 0xC) = arg2;
}
