/* No attempt: the row starts with a linker remnant. 268 bytes at 0x38AA20 begin
   `addiu sp,sp,+0x18` / nop; the real body starts at 0x38AA38 and the first 0x18 bytes
   belong to what precedes it. A tools/levels.py split is what unblocks it. */
