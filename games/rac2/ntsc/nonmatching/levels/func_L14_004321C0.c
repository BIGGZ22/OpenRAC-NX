/* func_L14_004321C0 (16 bytes, 1 place) -- NEAR MISS, not integrated: the row is not one
   function. The sixteen bytes are two complete eight-byte stubs, `jr $ra` + `nop` at
   0x004321C0 and again at 0x004321C8, and the catalogue merged them into one row:
     004321B4  jr    $ra                 <- end of the function before it
     004321B8  addiu $sp, $sp, 0x290     <- its frame restore, in the delay slot
     004321BC  nop                       <- alignment padding
     004321C0  jr    $ra                 } stub 1, the row's real start
     004321C4  nop                       }
     004321C8  jr    $ra                 } stub 2, a second function of its own
     004321CC  nop                       }
     004321D0  addiu $sp, $sp, -0x20     <- func_L14_004321D0 (636 bytes)
   C can only produce eight bytes for an empty function, so the row cannot match at any
   spelling (check.py would say "8 bytes, expected 16"). Fix belongs in tools/levels.py: a
   16-byte row of two stubs is two entries, or the row starts at 0x004321C8.
   The address right after the padding nop is the row's real body: 0x004321C0, 8 bytes. */
