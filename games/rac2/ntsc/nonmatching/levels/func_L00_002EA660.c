/* No attempt: the retail bytes are not code. The four-byte extent at
   0x002EA660 is 0xAC600014 - a bare `sw $zero, 0x14($v1)` with no prologue,
   no body and no `jr $ra` - and the four bytes after it are a `nop`. It
   sits between the previous function's `jr $ra` and its epilogue and the
   next function's prologue at 0x002EA668. The same shape as
   func_L00_00341798, func_L01_002E1670 and func_L03_002B0958: a catalogue
   split error, not a function. No C can be placed. */
