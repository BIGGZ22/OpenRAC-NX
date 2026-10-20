/* func_L16_0041F950 - 368 bytes, 2 place(s), level 16_snivelak, retail at
   0x0041f950. Screened with tools/w10_screen.py: 4 call(s)
   [func_002FC560@0041F998, func_0027DE50@0041F9BC,
   func_L04_00459408@0041FA34, func_L04_00459408@0041FAA4]; data addresses
   materialised [0x1a0000, 0x80800000, 0x3dcc0000, 0x3dcc0000]; float
   constants [0x3f000000, 0x3f000000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [COP1
   46800020@0x00a0, COP1 46800020@0x0114]. Head: addiu $sp, $sp, -0x20 ; sd
   $s1, 0x8($sp) ; sd $ra, 0x10($sp) ; daddu $s1, $a0, $zero ; sd $s0,
   0x0($sp) ; lw $s0, 0x68($s1). Not matched in this wave: the float code
   has to come out of the same expression tree to keep the FPU registers.
   What would unblock it: the exact source expression tree, so this cc1
   picks the same registers in the same order. */
