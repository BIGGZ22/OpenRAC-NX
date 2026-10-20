/* Twelve bytes, `addiu $sp,$sp,0x30; nop; addiu $sp,$sp,0x50`, no `jr $ra` and
   nothing that sets a register it reads. The twelve bytes before it are the end of
   the previous function (`jr $ra; addiu $sp,$sp,0x50` at 0x0032E890..0x0032E894),
   so the row is a split-off remnant between two entries, not C.
   Real entry of the next body is elsewhere; the row is one place only (17_smolg).
   Fix belongs in tools/levels.py. Kept as data for the maintainers. */
