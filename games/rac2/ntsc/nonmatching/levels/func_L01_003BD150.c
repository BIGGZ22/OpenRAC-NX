/* func_L01_003BD150 (28 bytes, 1 place) -- NEAR MISS, not integrated: the row is not code.
   Twenty-eight bytes is not a multiple of eight, which no function body can be, and the
   four words it holds are data read as instructions:
     003BD140  addiu $sp, $sp, 0x50      <- the epilogue of the function before it
     003BD144  nop
     003BD148  jr    $ra
     003BD14C  nop
     003BD150  .word 0x27BD0020          } the row: three `addiu $sp,$sp,0x20` + nop
     003BD158  .word 0x27BD0020          } pairs and one `addiu $sp,$sp,0x10` + nop,
     003BD160  .word 0x27BD0020          } i.e. a small table of frame sizes
     003BD168  .word 0x27BD0010          }
     003BD16C  nop                       <- padding
     003BD170  nop
     003BD174  nop
     003BD178  addiu $sp, $sp, -0xE0     <- the next function's prologue
   Nothing here can be placed from C: the real body resumes at 0x003BD178, and the row is a
   data blob sitting in the catalogue's code range (fix in tools/levels.py). */
