/* func_L05_0033D3A0 - 1848 bytes, 5 place(s), level 05_feltzin_system,
   retail at 0x0033d3a0. Screened with tools/w10_screen.py: 30 call(s)
   [func_002B1A80@0033D4A4, func_002B1A80@0033D4D8, func_002B1A80@0033D584,
   func_002B1A80@0033D5B8, func_002A7470@0033D628, func_002B1A80@0033D66C,
   func_002B1A80@0033D6D4, func_002A7470@0033D6EC, func_002A7470@0033D73C,
   func_002B1A80@0033D7A4, func_002A7470@0033D7BC, func_002A7470@0033D82C,
   func_002AB800@0033D864, func_002AFF38@0033D898, func_002AFFD8@0033D8BC,
   func_00282F20@0033D8D4, func_002AE730@0033D8E0, func_00282DB0@0033D8EC,
   func_00282F20@0033D900, func_00282CF0@0033D910, func_00282DF0@0033D918,
   func_00282F20@0033D934, func_00282D30@0033D948, func_00282F20@0033D960,
   func_00282CC0@0033D970, func_002B1CC8@0033D9A8, func_00282E20@0033D9B0,
   func_00282F70@0033D9CC, func_00283248@0033DA18, func_00283B98@0033DA2C];
   data addresses materialised [0x2c0000, 0x3dd60000, 0x3ed60000,
   0x3f490000, 0x40490000]; float constants [0x3f000000, 0x3f400000,
   0x3fc00000, 0x3f800000, 0xbf800000, 0x40a00000, 0x40800000, 0x3f800000,
   0xbf800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [COP1 468018E0@0x0184,
   COP1 468018E0@0x0268, COP1 468010A0@0x0278, COP1 46800860@0x0380, COP1
   46800020@0x0390, COP1 468018E0@0x046c, COP1 468010A0@0x047c]. Head: addiu
   $sp, $sp, 0x60 ; nop ; addiu $sp, $sp, -0xE0 ; addiu $v0, $zero, 0x1 ; sd
   $s0, 0x80($sp) ; sd $s1, 0x88($sp). Not matched in this wave: the float
   code has to come out of the same expression tree to keep the FPU
   registers. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
