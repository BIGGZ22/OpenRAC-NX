/* No attempt: the row starts with a linker remnant. 52 bytes at 0x3383B8 begin
   `addiu sp,sp,+0x10` / nop; the real body starts at 0x3383E0, and the row carries no
   `jr $ra` (the extent runs into the next function).
   A tools/levels.py split is what unblocks it. */
