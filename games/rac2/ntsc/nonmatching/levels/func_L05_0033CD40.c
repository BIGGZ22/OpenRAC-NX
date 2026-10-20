/* Not a C row: a debris mix, not one function - `addiu $sp,$sp,0x30; nop; addiu $sp,$sp,0x30;
   nop; sh $zero,0x1C($a0); nop; addiu $sp,$sp,0x30; nop`, level 05 (6 places). The row opens on
   stack restores (impossible as a prologue) and the lone `sh` is a stray body word; the previous
   row func_L00_00326048 ends at 0x33CD3C and the real entry after the gap is func_002B0A18 at
   0x33CD60.
   Maintainer data: no C spelling; 28 dead bytes. */
