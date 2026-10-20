/* func_L00_00385370 - 720 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00385370. Screened with tools/w10_screen.py: 22 call(s)
   [func_00303210@0038541C, func_002A7878@00385480, func_002A7878@003854A0,
   func_002A7750@003854BC, func_002B3868@003854CC, func_00282948@003854E8,
   func_002A7750@003854FC, func_002B3868@00385514, func_00282948@00385534,
   func_002A7750@00385548, func_002B3868@00385558, func_00282948@00385574,
   func_002A7750@00385588, func_002AA548@003855A4, func_002B3868@003855B4,
   func_00282948@003855D0, func_002A7750@003855E4, func_00303BF8@003855F8,
   func_00303AC8@00385600, func_00302DA0@00385608, func_00304128@00385610,
   func_L00_002E6DB8@00385620]; data addresses materialised [0x2a0000,
   0x230000, 0x3b030000, 0x3bc40000, 0x380000]; float constants [0x40800000,
   0x3f800000, 0x3f000000, 0x40000000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [COP1
   468010A0@0x0080, COP1 468018E0@0x008c]. Head: addiu $sp, $sp, -0x20 ; sd
   $s2, 0x10($sp) ; sd $s0, 0x0($sp) ; daddu $s2, $a0, $zero ; sd $s1,
   0x8($sp) ; sd $ra, 0x18($sp). Not matched in this wave: the float code
   has to come out of the same expression tree to keep the FPU registers.
   What would unblock it: the exact source expression tree, so this cc1
   picks the same registers in the same order. */
