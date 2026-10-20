/* Not a C row: the extent opens on a stack-restore fragment.
   Retail at 0x430A90, level 00 (same bytes in all 25 places): `addiu $sp,$sp,0x30; nop;
   addiu $sp,$sp,0x30` with a `nop` after the extent at 0x430A9C; the previous entry ends at
   0x430A90 with `jr $ra; addiu $sp,$sp,0x30` and the next row is the boot function func_0033B170 at
   0x430AA0 (964 bytes). Nothing can start with `addiu $sp,$sp,+0x30`, so this is a catalogue split,
   not a function - the same class as func_L00_00358670 in this lot.
   Maintainer data: no C spelling; absorb into the neighbour or drop the row. */
