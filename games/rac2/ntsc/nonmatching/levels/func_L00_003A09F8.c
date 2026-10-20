/* func_L00_003A09F8 - 520 bytes, 17 place(s), level 00_aranos_tutorial,
   retail at 0x003a09f8. Screened with tools/w10_screen.py: 11 call(s)
   [func_00283CE0@003A0A44, func_00283CE0@003A0A54, func_0027C878@003A0A60,
   func_00283CE0@003A0B14, func_00283CE0@003A0B20, func_00283CF0@003A0B30,
   func_002A77E0@003A0B40, func_002A77E0@003A0B4C, func_00283CF0@003A0B5C,
   func_00283CF0@003A0B6C, func_0027E018@003A0B9C]; data addresses
   materialised [0x1a0000, 0x1a0000, 0x1b0000]; float constants [0x1a0000,
   0x1a0000, 0xff0000, 0x42ff0000]; $gp-relative small data: no   saves: -;
   callee-saved registers saved [-]; words plain C cannot emit [COP1
   4680B5A0@0x00d8, COP1 46800020@0x00fc, COP1 4680AD60@0x0110]. Head: addiu
   $sp, $sp, -0xB0 ; sd $s6, 0x70($sp) ; sd $fp, 0x80($sp) ; sd $s0,
   0x40($sp) ; sd $s1, 0x48($sp) ; sd $s2, 0x50($sp). Not matched in this
   wave: the float code has to come out of the same expression tree to keep
   the FPU registers. What would unblock it: the exact source expression
   tree, so this cc1 picks the same registers in the same order. */
