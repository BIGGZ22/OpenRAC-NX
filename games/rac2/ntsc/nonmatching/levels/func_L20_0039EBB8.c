/* Not matched (worker 06, level-20 wave of 2026-10-20): the 128-bit
   copies are the wall.  The retail keeps the address of every 128-bit
   access in a register - `lq $v0,0x0($s2)` with `addiu $s2,$s1,0x10`
   (the object's +0x10), `lq $v0,0x0($s3)` with `addiu $s3,$s1,0xF0`
   (the object's +0xF0), `lq $v0,0x0($v1)` with `addiu $v1,$sp,0x10`
   (a local buffer) - while this compiler folds the offset into the
   instruction (`lq $v0,0x10($s1)`).

   The mechanism is measured in the note of func_L20_0039C448: the CSE
   substitutes the pointer's value (`plus(reg,const)`) into the address
   of the `lq`/`sq` and the reload pass then keeps the offset in the
   instruction; a pointer that is a parameter or a value loaded from
   memory keeps the register form, a computed `reg + const` never does.
   Every source spelling measured folds: a TI128 pointer variable, a
   struct member, an array index, a volatile load, a volatile pointer
   (a stack store and a reload, eight bytes over), a `static
   __inline__` copy helper, a truncation and a 64-bit address.

   This body belongs in the `-G8` unit?  No: it reaches no global
   through `$gp`. */
