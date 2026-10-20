/* func_L00_0041DEF0 - 632 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041def0. Screened with tools/w10_screen.py: 19 call(s)
   [func_00282CF0@0041DF28, func_00282DF0@0041DF30, func_002ACF68@0041DF78,
   func_00283CE0@0041DFAC, func_00283CE0@0041DFB8, func_00282DD8@0041DFD8,
   func_00282DB0@0041DFE4, func_00282DF0@0041DFF0, func_00282DF0@0041DFFC,
   func_002831B0@0041E024, func_00282C48@0041E040, func_00282DD8@0041E070,
   func_00282DD8@0041E080, func_00282F20@0041E098, func_002703B0@0041E0D0,
   func_00283198@0041E104, func_00282D30@0041E114, func_00283180@0041E11C,
   func_002ACDE8@0041E130]; data addresses materialised [0x3fc90000,
   0x40380000, 0x3ccc0000, 0x3e990000, 0x42650000]; float constants
   [0x3fc00000, 0x3f000000, 0x3f800000, 0x3f000000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [-]. Head: addiu $sp, $sp, -0x80 ; sd $s3, 0x48($sp) ; swc1
   $f22, 0x70($sp) ; daddu $s3, $a2, $zero ; sd $s0, 0x30($sp) ; addiu $a2,
   $a0, 0x10. Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
