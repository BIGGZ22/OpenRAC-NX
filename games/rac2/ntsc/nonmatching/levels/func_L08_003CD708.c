/* func_L08_003CD708 - 1068 bytes, 3 place(s), level 08_tabora, retail at
   0x003cd708. Screened with tools/w10_screen.py: 15 call(s)
   [func_002AA3A8@003CD868, func_00283BE0@003CD898, func_002AA3A8@003CD8B8,
   func_00283BE0@003CD8E8, func_00282C48@003CD944, func_00282C48@003CD950,
   func_002AA2E8@003CD970, func_00282C48@003CD9A8, func_00283BE0@003CD9D4,
   func_002AA2E8@003CDA40, func_00283B98@003CDA4C, func_002AA898@003CDA88,
   func_002AA3A8@003CDAA4, func_002AA898@003CDAD4, func_00283BE0@003CDB04];
   data addresses materialised [0x140000, 0x39980000, 0x39980000,
   0x3c8e0000, 0x190000, 0x3c8e0000, 0x3e4c0000, 0x39980000, 0x36a20000,
   0x3c340000, 0x3e990000, 0x3c6e0000, 0x39980000]; float constants
   [0xc0000000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x40 ; mov.s $f5, $f12 ; sd $s0, 0x0($sp) ; addiu $v0, $zero, 0x8 ; swc1
   $f22, 0x30($sp) ; daddu $s0, $a1, $zero. Not matched in this wave: the
   body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
