/* func_L00_0033E210 - the catalogue row is a stripped epilogue fragment, not a function:
   twelve bytes `addiu $sp,$sp,0x70; nop; addiu $sp,$sp,0x30`, no `jr $ra` anywhere, identical
   in every place checked (00_aranos_tutorial 0x0033E210, 01_oozla 0x00334240). The two `addiu`
   are the stack restores of two neighbouring functions' epilogues.
   Needs a tools/levels.py split, like the other rows that open on `addiu $sp,$sp,+N; nop`. */
