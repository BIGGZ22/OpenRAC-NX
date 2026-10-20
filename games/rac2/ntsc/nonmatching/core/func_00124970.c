/* from src/core/00124568.c: the row is not a function body - it is 12 bytes of linker
   remnant, and the catalogue gives it a name all the same.

     0x124970  addiu $sp,$sp,0xC0   the surviving delay slot of the `jr $ra` of a
                                   function the linker dead-stripped (its frame is
                                   0xC0, and the order is the strip_dead model
                                   rac1-decomp documents);
     0x124974  nop                  that remnant's alignment nop;
     0x124978  addiu $sp,$sp,0x10   the delay slot of a second stripped function
                                   (0x10 frame);
     0x12497C  nop                  its alignment nop, outside the row.

   The row therefore holds the first three words of a 16-byte remnant run; the real
   function starts at 0x124980, where func_00124980 (`_sceCd_scmd_prechk`) already sits
   as C in src/core/00124568.c. rac1-decomp keeps the same remnant with LINKER_REMNANT as
   func_00120D18, in front of its func_00120D28.

   Nothing reaches these words: the row has no `jr $ra` and no branch at all, and no
   direct `j`/`jal` anywhere in the boot executable targets 0x124970..0x12497F. A
   function's body it is not, so no C can be written for it; what would clear the row is
   a tools/levels.py split that hands 0x124970..0x12497F to a filler row (func_00124980
   already has its own row at 0x124980), as the catalogue already does elsewhere for
   remnants. No compile was spent: the offset is arithmetic, not a source shape. */
