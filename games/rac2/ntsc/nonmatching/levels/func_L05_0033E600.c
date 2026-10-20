/* Not a C row: the extent is a stack of restores, not a function.
   Retail at 0x33E600, level 05 (9 places): `addiu $sp,$sp,0x90; nop; addiu $sp,$sp,0x150; nop;
   addiu $sp,$sp,0x10`. Three consecutive frame restores with their padding; the same class as
   func_L00_00358670 in this lot. No function opens with `addiu $sp,$sp,+N`.
   Maintainer data: no C spelling; absorb into the neighbour or drop the row. */
