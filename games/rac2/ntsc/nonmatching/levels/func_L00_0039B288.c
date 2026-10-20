/* func_L00_0039B288 - 440 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0039b288. Screened with tools/w10_screen.py: 11 call(s)
   [func_00273B70@0039B2C0, func_002A77E0@0039B2F8, func_002A7750@0039B310,
   func_00273B70@0039B328, func_0029F338@0039B340, func_00282DB0@0039B35C,
   func_00282C48@0039B364, func_00282C30@0039B38C, func_002A77E0@0039B3C8,
   func_00283D38@0039B404, func_002A0488@0039B420]; data addresses
   materialised [0x1c0000, 0x1b0000, 0x3f4c0000, 0x1c0000, 0x1c0000,
   0x1c0000, 0x3dcc0000, 0x3f4c0000]; float constants [-]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [COP1 46800020@0x012c]. Head: addiu $sp, $sp, -0x30 ; sd
   $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; daddu $s0, $a0, $zero ; sd $ra,
   0x10($sp) ; swc1 $f20, 0x20($sp). Not matched in this wave: the float
   code has to come out of the same expression tree to keep the FPU
   registers. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
