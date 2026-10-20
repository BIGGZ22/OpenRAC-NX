/* Not a C row: the extent opens on a dead fragment between two functions.
   Retail at 0x37B6C0, level 05 (4 places): `lw $v0,0xC($v1)` with $v1 never set, then `nop;
   addiu $sp,$sp,0x20`. The row before it ends at 0x37B6C0 (`jr $ra; addiu $sp,$sp,0x20`) and the
   row after it, func_L05_0037B6D0 at 0x37B6D0, is a real function with a proper prologue.
   Same class as func_L00_00358670 in this lot.
   Maintainer data: no C spelling; absorb into the neighbour or drop the row. */
