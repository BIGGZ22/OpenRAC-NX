/* Not attempted: the body holds the level's 128-bit zero-store wall - the
   retail materialises the zero of the 16-byte stack buffer
       por $v0, $zero, $zero ; sq $v0, 0x20($sp)   (0x00449EEC)
   where this cc1 folds it to `sq $zero, 0x20($sp)`, the same wall as
   func_L20_0044AB68 (see that file for the measurement).  The 16-byte copy
   at 0x00449F08 is not a wall: both addresses are stack offsets there
   (lq $v0,0x20($sp) ; sq $v0,0x10($sp)). */
