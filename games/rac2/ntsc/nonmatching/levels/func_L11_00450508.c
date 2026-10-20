/* func_L11_00450508 - 440 bytes, 2 place(s), level 11_joba, retail at
   0x00450508. Screened with tools/w10_screen.py: 12 call(s)
   [func_002A7820@00450564, func_002A7940@0045056C,
   func_L11_004507B0@00450598, func_002A7820@004505B4,
   func_002A7940@004505BC, func_L11_004507B0@004505E8,
   func_002A7820@00450614, func_002A7940@0045061C,
   func_L11_004507B0@00450648, func_002A7820@00450664,
   func_002A7940@0045066C, func_L11_004507B0@00450698]; data addresses
   materialised [0x100000, 0x3f0000, 0x100000, 0x3f0000]; float constants
   [0x42340000, 0x3f800000, 0x41b40000, 0x42340000, 0x3f800000, 0x41b40000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x30 ; sd $s2,
   0x10($sp) ; sd $s0, 0x0($sp) ; daddu $s2, $a0, $zero ; sd $s1, 0x8($sp) ;
   sd $ra, 0x18($sp). Not matched in this wave: the body is straight-line
   code whose register allocation and the materialisation of its constant
   addresses have to be reproduced word for word. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
