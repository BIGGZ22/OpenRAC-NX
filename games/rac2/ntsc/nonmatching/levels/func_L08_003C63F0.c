/* func_L08_003C63F0 - 308 bytes, 3 place(s), level 08_tabora, retail at
   0x003c63f0. Screened with tools/w10_screen.py: 3 call(s)
   [func_00282E80@003C646C, func_00283CF0@003C64B4, func_00283CF0@003C64E0];
   data addresses materialised [0x1b0000]; float constants [0x40a00000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [COP1 46800020@0x00ac, COP1 46800020@0x00dc].
   Head: addiu $sp, $sp, -0x50 ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; lui
   $s0, 0x1B ; lw $s0, 0x29E8($s0) ; daddu $s1, $a0, $zero. Not matched in
   this wave: the float code has to come out of the same expression tree to
   keep the FPU registers. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
