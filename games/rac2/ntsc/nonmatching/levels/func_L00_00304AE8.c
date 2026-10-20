/* func_L00_00304AE8 - the catalogue row starts eight bytes early in every place.
   +0x0 `addiu $sp,$sp,0x10` and +0x4 `nop` are the previous function's return delay slot and
   its alignment padding, not this body: what follows (+0x8 `beqz $a1` ... `jr $ra` with
   `sw $v0,0x0($a2)` in the slot) is a complete function that reads `lw $a1,-0x5974($gp)`.
   Real entry = row + 8 in each place (00_aranos_tutorial 0x00304AF0, 01_oozla 0x002FA520,
   25_wupash_nebula 0x00317218), size 76 bytes. Needs a tools/levels.py split; the C body for
   the real entry is what a maintainer should check against the row. */
