/* No attempt: the retail bytes are not code. The four-byte extent at
   0x002D7138 is 0xAC820038 - a bare `sw $v0, 0x38($a0)` with no prologue,
   no body and no `jr $ra` - and the four bytes after it are a `nop`. It
   sits between the previous function's `jr $ra` and its epilogue and the
   next function's prologue at 0x002D7140. The same shape as
   func_L00_00341798, func_L01_002E1670 and func_L03_002B0958: a catalogue
   split error, not a function. No C can be placed. */
