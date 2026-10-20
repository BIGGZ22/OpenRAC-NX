/* 0 of 2 places match - the extent is linker fill, not a function.
   Both placements (03_endako 0x0038B5A8, 14_aranos_prison 0x00391B98) hold
   `addiu $sp,$sp,0x80 ; nop` followed by five `addiu $sp,$sp,0x10 ; nop` pairs:
   no `jr $ra` anywhere and no prologue.  The next real bodies are
   func_L03_0038B5D8 at 0x0038B5D8 and 0x00391BC8.  A tools/levels.py split
   unblocks it; no C reproduces linker fill. */
