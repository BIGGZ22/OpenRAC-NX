/* func_L13_004388C8 - 1212 bytes, 3 place(s), level 13_boldan, retail at
   0x004388c8. Screened with tools/w10_screen.py: 10 call(s)
   [func_00282C88@004389D0, func_00282C88@00438A08, func_00282C88@00438A18,
   func_00282C88@00438A20, func_L00_00341FA8@00438B2C,
   func_00283CF0@00438B78, func_00283CF0@00438B88,
   func_L00_00341FA8@00438C5C, func_00283CF0@00438CA8,
   func_00283CF0@00438CB8]; data addresses materialised [0x3e560000,
   0x3a030000, 0x3e330000, 0x3c230000, 0x3e4c0000, 0x3e990000, 0x3b440000,
   0x3dcc0000, 0x200000, 0x1b0000]; float constants [0x3fc00000, 0x1b0000,
   0x3f000000, 0x40c00000, 0x3f000000, 0x40000000, 0x40400000, 0x41a00000,
   0x40a00000, 0x41a00000, 0x40a00000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [COP1
   46800860@0x027c, COP1 46800860@0x03ac]. Head: addiu $sp, $sp, -0x80 ; lui
   $at, 0x3FC0 ; mtc1 $at, $f0 ; sd $s2, 0x30($sp) ; addiu $a1, $zero, 0x3 ;
   sd $s0, 0x20($sp). Not matched in this wave: the float code has to come
   out of the same expression tree to keep the FPU registers. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
