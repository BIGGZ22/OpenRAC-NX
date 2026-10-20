/* func_L00_00421B50 - 252 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00421b50. Screened with tools/w10_screen.py: 3 call(s)
   [func_00282978@00421B78, func_L00_00421A98@00421BB4,
   func_L00_004219D8@00421C28]; data addresses materialised [-]; float
   constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [COP1 46800020@0x0058].
   Head: addiu $sp, $sp, -0x40 ; sd $s0, 0x10($sp) ; sd $s1, 0x18($sp) ; sd
   $s2, 0x20($sp) ; daddu $s1, $a1, $zero ; sd $ra, 0x28($sp). Not matched
   in this wave: the float code has to come out of the same expression tree
   to keep the FPU registers. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
