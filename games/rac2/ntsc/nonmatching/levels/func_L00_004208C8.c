/* func_L00_004208C8 - 720 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x004208c8. Screened with tools/w10_screen.py: 19 call(s)
   [func_00282CC0@004209AC, func_00282CC0@004209C4, func_00282CC0@004209EC,
   func_00282CC0@004209FC, func_00282CC0@00420A0C, func_00282CC0@00420A1C,
   func_00282DB0@00420A28, func_00282D30@00420A3C, func_00282CC0@00420A4C,
   func_00282D30@00420AD0, func_00282DB0@00420ADC, func_00282D30@00420AF0,
   func_00282CF0@00420B00, func_00282CF0@00420B10, func_00282CC0@00420B20,
   func_00282CC0@00420B30, func_00282CC0@00420B40, func_00282CC0@00420B50,
   func_00282CC0@00420B60]; data addresses materialised [0x190000, 0x1b0000,
   0x1c0000, 0x1c0000, 0x1c0000]; float constants [-]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 78A30000@0x0044, LQ/SQ 7FA30000@0x0054]. Head: addiu
   $sp, $sp, -0xA0 ; lui $v0, 0x19 ; sd $s3, 0x68($sp) ; addiu $a2, $v0,
   -0x61E0 ; sd $s4, 0x70($sp) ; sd $s5, 0x78($sp). Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
