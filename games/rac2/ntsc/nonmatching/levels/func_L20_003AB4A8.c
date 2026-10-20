/* Not matched (worker 06, level-20 wave of 2026-10-20): the 128-bit
   copies are the wall.  The retail mixes the two forms inside this
   one body: the stores to the stack keep their offset (`sq $v0,0x10($sp)`
   - the offset is in the instruction) while the copies whose base is a
   computed pointer go through a register (`lq $v0,0x0($s3)` with `addiu
   $s3,...`, `sq $v0,0x0($s5)`, `lq $v0,0x0($s2)`, `sq $v0,0x0($s0)`,
   `lq $v0,0x0($a0)`, `sq $v0,0x0($v1)`).

   This compiler folds the offset of every 128-bit access into the
   instruction, so the copies whose base is `reg + const` do not match.
   The mechanism is measured in the note of func_L20_0039C448: the CSE
   substitutes the pointer's value into the address of the `lq`/`sq` and
   the reload pass keeps the offset in the instruction; only a pointer
   that is a parameter or a value loaded from memory keeps the register
   form.

   This body belongs in the `-G8` unit?  No: it reaches no global
   through `$gp`. */
