/* func_L00_00396078 - 528 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00396078. Screened with tools/w10_screen.py: 16 call(s)
   [func_00282D30@003960AC, func_00282CC0@003960C0, func_0027C878@003960C8,
   func_002A77E0@00396120, func_00283CE0@00396138, func_00283CF0@0039614C,
   func_00282CF0@00396190, func_00282F20@003961A4, func_00282DD8@003961BC,
   func_00282F20@003961D0, func_00282DD8@003961E0, func_00282D30@003961F8,
   func_00282CC0@00396208, func_00282D30@00396228, func_002830C0@00396240,
   func_00280B90@00396258]; data addresses materialised [0x1b0000, 0xff0000,
   0x1c0000, 0x190000, 0x3e4c0000]; float constants [0x3f800000, 0x3f800000,
   0xbf800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x140 ; lwc1 $f12, 0x29C($gp) ; sd $s0, 0x100($sp) ; sd $s1, 0x108($sp)
   ; addiu $s0, $sp, 0xE0 ; daddu $s1, $a0, $zero. Not matched in this wave:
   the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
