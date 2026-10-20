/* func_L00_00382CE8 - 572 bytes, 4 place(s), level 00_aranos_tutorial,
   retail at 0x00382ce8. Screened with tools/w10_screen.py: 10 call(s)
   [func_002A77E0@00382D58, func_002A77E0@00382D64, func_002A77E0@00382DC8,
   func_002A77E0@00382DE4, func_002A77E0@00382DF0, func_002A77E0@00382E14,
   func_002A7878@00382E58, func_00283C28@00382E70, func_00283180@00382E78,
   func_00282C48@00382E80]; data addresses materialised [0x1c0000, 0x200000,
   0x40190000, 0xbf730000, 0x3fb30000, 0x800000]; float constants
   [0x3f800000, 0xbf800000, 0x40000000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [-]. Head: addiu $sp, $sp, -0x50 ; lui $v0, 0x1C ; sd $s0, 0x0($sp) ;
   daddu $a2, $v0, $zero ; sd $s1, 0x8($sp) ; addiu $v0, $v0, -0x6F40. Not
   matched in this wave: the body is straight-line code whose register
   allocation and the materialisation of its constant addresses have to be
   reproduced word for word. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
