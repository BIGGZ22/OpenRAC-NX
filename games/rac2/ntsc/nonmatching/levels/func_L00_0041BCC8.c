/* func_L00_0041BCC8 - 1176 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041bcc8. Screened with tools/w10_screen.py: 19 call(s)
   [func_00283CF0@0041BD4C, func_L00_0041C160@0041BD88,
   func_L00_0041C160@0041BDC0, func_0027C878@0041BE68,
   func_00282CF0@0041BF50, func_002AFFD8@0041BF60, func_00282F20@0041BF70,
   func_00282DD8@0041BF80, func_00282F20@0041BF90, func_00282DD8@0041BFA0,
   func_00282D30@0041BFDC, func_00282D30@0041BFEC, func_00282D30@0041BFFC,
   func_00282D30@0041C00C, func_00282D30@0041C020, func_00282D30@0041C030,
   func_00282D30@0041C040, func_00282D30@0041C050, func_00280B90@0041C064];
   data addresses materialised [0x100000, 0x100000, 0x3eb30000, 0xff0000,
   0x3d990000, 0x100000, 0x1c0000]; float constants [0x42800000, 0x3f800000,
   0x3f000000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x110 ; sd $s3, 0xC8($sp) ; sd $s0, 0xB0($sp) ; daddu $s3, $zero, $zero
   ; sd $s1, 0xB8($sp) ; sd $s2, 0xC0($sp). Not matched in this wave: the
   body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
