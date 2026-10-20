/* func_L13_00439820 - 588 bytes, 3 place(s), level 13_boldan, retail at
   0x00439820. Screened with tools/w10_screen.py: 8 call(s)
   [func_00282E48@00439904, func_L00_00341FA8@00439990,
   func_00282CF0@004399D0, func_00282DF0@004399D8, func_00282F20@004399FC,
   func_00282CF0@00439A10, func_00282DB0@00439A1C,
   func_L13_004388C8@00439A38]; data addresses materialised [0x1b0000,
   0x1c0000, 0x200000, 0x200000, 0x190000]; float constants [0x3f800000,
   0x41a00000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 78820000@0x0054,
   LQ/SQ 7CC20000@0x0058, LQ/SQ 78A20000@0x005c, LQ/SQ 7E620000@0x0060,
   LQ/SQ 78620000@0x006c, LQ/SQ 7C820000@0x0070, LQ/SQ 78A20000@0x007c,
   LQ/SQ 7C620000@0x0080, LQ/SQ 7A620000@0x0088, LQ/SQ 7C820000@0x008c].
   Head: addiu $sp, $sp, -0x90 ; lui $a3, 0x1B ; lw $a3, 0x22C4($a3) ; sd
   $s3, 0x58($sp) ; lui $v0, 0x1C ; sd $s0, 0x40($sp). Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
