/* func_L18_002CFF48 - 504 bytes, 2 place(s), level 18_damosel, retail at
   0x002cff48. Screened with tools/w10_screen.py: 12 call(s)
   [func_002AB1F0@002CFF78, func_0029B758@002CFFC8,
   func_L00_002B2BD0@002CFFD0, func_L00_002A53B8@002D0000,
   func_0028B3D0@002D0030, func_L00_002F6B90@002D0038,
   func_L00_002A6708@002D0058, func_L00_002D24B8@002D0084,
   func_L00_002A4DB0@002D00A8, func_L00_002A4DB0@002D00C8,
   func_L18_002E0070@002D0104, func_L00_002D9D98@002D0120]; data addresses
   materialised [0x190000, 0x310000, 0x310000, 0x310000, 0x1b0000,
   0x1b0000]; float constants [0x3f800000, 0x1b0000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [COP1 46800020@0x0044, LQ/SQ 78620000@0x0058, LQ/SQ
   7C820000@0x005c, LQ/SQ 78A20000@0x0068, LQ/SQ 7C620000@0x006c, LQ/SQ
   78A20000@0x0128, LQ/SQ 7C620000@0x012c]. Head: addiu $sp, $sp, -0x20 ;
   lui $v0, 0x19 ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; addiu $s0, $v0,
   -0x61E0 ; sd $s2, 0x10($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
