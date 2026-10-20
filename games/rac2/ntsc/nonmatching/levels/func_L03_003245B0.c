/* No attempt: the retail bytes are not code. Three words at 0x3245B0: 0x27BD0030, 0,
   0x27BD0060 (addiu sp,sp,0x30 / nop / addiu sp,sp,0x60), sitting between the epilogue of
   the function ending at 0x3245AC and the next function's prologue at 0x3245C0.
   Same shape as func_L00_00341798: a data table or a catalogue split error; no C can be
   placed. */
