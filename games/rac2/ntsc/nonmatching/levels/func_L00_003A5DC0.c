/* func_L00_003A5DC0 - 508 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a5dc0. Screened with tools/w10_screen.py: 12 call(s)
   [func_00282CF0@003A5DF4, func_00282F20@003A5E08, func_002761F8@003A5E34,
   func_00282CF0@003A5E48, func_00282F20@003A5E5C, func_00282CF0@003A5E74,
   func_L00_003FC108@003A5E88, func_00282E48@003A5EC0,
   func_002A8E20@003A5F50, func_0032D758@003A5F60, func_00282CF0@003A5F7C,
   func_L00_003FC108@003A5F90]; data addresses materialised [0x45af0000,
   0x190000, 0x1c0000, 0x1c0000, 0x140000, 0x260000, 0x1c0000]; float
   constants [0x3fc00000, 0x3f800000, 0x41200000, 0x41600000, 0x3f400000,
   0x41a00000, 0x3e000000, 0x3f000000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [-]. Head:
   addiu $sp, $sp, -0x50 ; sd $s0, 0x10($sp) ; sd $s1, 0x18($sp) ; daddu
   $s0, $a1, $zero ; sd $s2, 0x20($sp) ; daddu $s1, $a2, $zero. Not matched
   in this wave: the body is straight-line code whose register allocation
   and the materialisation of its constant addresses have to be reproduced
   word for word. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
