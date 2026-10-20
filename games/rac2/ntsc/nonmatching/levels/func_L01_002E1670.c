/* No attempt: the retail bytes are not code. Three words at 0x2E1670: 0x27BD00C0, 0,
   0x27BD00F0 (addiu sp,sp,0xC0 / nop / addiu sp,sp,0xF0), sitting between the epilogue of
   the function ending at 0x2E166C and the next function's prologue at 0x2E1680.
   Same shape as func_L00_00341798: a data table or a catalogue split error; no C can be
   placed. */
