/* func_L18_0035D238 - 880 bytes, 3 place(s), level 18_damosel, retail at
   0x0035d238. Screened with tools/w10_screen.py: 21 call(s)
   [func_00282CF0@0035D290, func_00282CF0@0035D2A4, func_00282DB0@0035D2B4,
   func_00282DB0@0035D2C4, func_00282D30@0035D2E8, func_00282CC0@0035D2F8,
   func_00282E48@0035D304, func_00282C10@0035D328, func_00282DF0@0035D334,
   func_00282D30@0035D3C0, func_00282CC0@0035D3D0, func_00282E48@0035D3DC,
   func_00282C48@0035D3E4, func_00282D30@0035D41C, func_00282CC0@0035D42C,
   func_00282E48@0035D438, func_00282C48@0035D440, func_00282E48@0035D484,
   func_00282E48@0035D494, func_00282E48@0035D51C, func_00282C48@0035D524];
   data addresses materialised [0x3c230000, 0x3c230000]; float constants
   [0x3f800000, 0x3f800000, 0x3f800000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 7A220000@0x0274, LQ/SQ 7E420000@0x0278, LQ/SQ 7A020000@0x0284,
   LQ/SQ 7E420000@0x0288, LQ/SQ 7A620000@0x02ac, LQ/SQ 7E420000@0x02b0,
   LQ/SQ 7AA20000@0x02d8, LQ/SQ 7E420000@0x02dc, LQ/SQ 7A220000@0x02fc,
   LQ/SQ 7E420000@0x0300]. Head: addiu $sp, $sp, -0xD0 ; sd $s2, 0x60($sp) ;
   sd $s3, 0x68($sp) ; daddu $s2, $a0, $zero ; sd $s4, 0x70($sp) ; daddu
   $s3, $a3, $zero. Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
