/* No attempt: the row starts with a linker remnant. 28 bytes at 0x329ED0 begin
   `addiu sp,sp,+0x10` / nop; the real body starts at 0x329EE8, and the row carries no
   `jr $ra` at all (the extent runs into the next function's prologue).
   Same shape as func_00127218 / func_001158F4; a tools/levels.py split is what unblocks
   it, that is not a worker's to make. */
