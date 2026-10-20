/* Not a C row: the extent opens on padding after the previous body, cut mid-pair.
   Retail at 0x002FE4C0, level 11 (1 place): twelve bytes read as `addiu $sp,$sp,0x80`, `nop`,
   `addiu $sp,$sp,0xC0` - both stack adjustments positive, so neither is a prologue, and there is
   no `jr $ra` at all. The rows either side fit exactly around it: func_L02_002EF968 ends at
   0x002FE4BC, func_L00_002EC248 begins at 0x002FE4D0, leaving the 4-byte align gaps at
   0x002FE4BC and 0x002FE4CC. Same class as func_L00_002ED7F8 (`addiu $sp,$sp,0x150; nop;
   addiu $sp,$sp,0x30`, documented in docs/COMPILER-NOTES.md) and as func_L05_0037B6C0 in this
   lot.
   Maintainer data: no C spelling; absorb into the neighbour or drop the row. */
