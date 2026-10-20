/* func_L05_00302CE8 - 788 bytes, 3 place(s), level 05_feltzin_system,
   retail at 0x00302ce8. Screened with tools/w10_screen.py: 3 call(s)
   [func_0027E018@00302EE8, func_0027E018@00302F30, func_0027E018@00302F7C];
   data addresses materialised [0xff0000]; float constants [-]; $gp-relative
   small data: yes   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [COP1 468010A0@0x00a8, COP1 46800020@0x00f4, COP1
   46800020@0x0120]. Head: addiu $sp, $sp, -0x70 ; sd $s1, 0x28($sp) ; daddu
   $s1, $t0, $zero ; sw $a0, 0x10($sp) ; sw $a1, 0x14($sp) ; dsll32 $v0,
   $s1, 0. Not matched in this wave: the float code has to come out of the
   same expression tree to keep the FPU registers. What would unblock it:
   the exact source expression tree, so this cc1 picks the same registers in
   the same order. */
