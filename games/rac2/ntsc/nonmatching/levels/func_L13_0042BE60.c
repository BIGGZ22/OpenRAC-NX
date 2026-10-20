/* No attempt: the row starts with a linker remnant. 44 bytes at 0x42BE60 begin
   `addiu sp,sp,+0x10` / nop, so the first 8 bytes are the previous function's epilogue
   fragment; a clean 32-byte body starts at 0x42BE68 and ends at 0x42BE88.
   A tools/levels.py split is what unblocks it. */
