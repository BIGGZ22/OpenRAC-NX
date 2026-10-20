/* Twelve bytes, `addiu $sp,$sp,0x30; nop; addiu $sp,$sp,0xD0`, no `jr $ra` and
   nothing that sets a register it reads. The twelve bytes before it are the end of
   the previous function (`ld $s4,0x30($sp); lwc1 $f20,0x40($sp); jr $ra;
   addiu $sp,$sp,0x50` ending at 0x0033D764), so this row is a split-off remnant
   between two entries, not C. One place only (18_damosel).
   Fix belongs in tools/levels.py. Kept as data for the maintainers. */
