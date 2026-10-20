/* Not a C row: the extent opens on a stack-restore fragment.
   Retail at 0x358670, level 00 (the same 12 bytes in all 27 places): `addiu $sp,$sp,0x10; nop;
   addiu $sp,$sp,0x10`, and the word after the extent is a `nop` at 0x35867C. The row before it,
   func_L00_00358668 (8 bytes), is a complete `jr $ra; addiu $v0,$zero,0x1`, and the entry after the
   gap is the boot function func_002DF1D0 at 0x358680 (28 bytes, `return *(int *)0x1FEC54 ==
   0x281E98`). No function can start with a stack restore, so the extent is a catalogue split - the
   same opener as L00_00430A90, L00_004263A8, L00_00428EE0, L05_0037B6C0, L05_0033E600 and
   L05_0033F4A8 in this lot.
   Maintainer data: the row has no C spelling; it should be absorbed by its neighbour or dropped. */
