/* Not a C row: same "addiu $sp,$sp,+N ; nop" opener as func_L00_00358670 in this lot.
   Retail at 0x33F4A8, level 05 (5 places) opens `addiu $sp,$sp,0x30; nop; addiu $sp,$sp,0x50; nop;
   addiu $sp,$sp,0x70` - 156 bytes of restores and padding. A function cannot begin by adding to the
   stack pointer, so the extent is a catalogue split.
   Maintainer data: no C spelling; the 156-byte row is dead bytes between real entries. */
