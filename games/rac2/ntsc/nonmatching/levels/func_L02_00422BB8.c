/* func_L02_00422BB8 - 428 bytes, 2 place(s), level 02_maktar_nebula, retail
   at 0x00422bb8. Screened with tools/w10_screen.py: 14 call(s)
   [func_L00_003DA2B0@00422BD8, func_0027C878@00422BE0,
   func_002A7820@00422C14, func_00283248@00422C64, func_00282E80@00422C74,
   func_00283248@00422C88, func_002A7940@00422C94, func_00283410@00422CA8,
   func_002A7878@00422CE8, func_00282D30@00422D00, func_00282D30@00422D10,
   func_00282D30@00422D20, func_00282D30@00422D30, func_00280B90@00422D40];
   data addresses materialised [0x1c0000, 0xff0000, 0x3ee60000, 0x3eb30000];
   float constants [0x3f800000]; $gp-relative small data: no   saves: -;
   callee-saved registers saved [-]; words plain C cannot emit [-]. Head:
   addiu $sp, $sp, -0x110 ; sd $s0, 0xE0($sp) ; sd $s1, 0xE8($sp) ; sd $ra,
   0xF8($sp) ; daddu $s1, $a0, $zero ; swc1 $f20, 0x100($sp). Not matched in
   this wave: the body is straight-line code whose register allocation and
   the materialisation of its constant addresses have to be reproduced word
   for word. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
