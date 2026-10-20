/* func_L00_0038D368 - 256 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038d368. Screened with tools/w10_screen.py: 7 call(s)
   [func_00283CE0@0038D39C, func_00283CE0@0038D3A8, func_00283B98@0038D3C8,
   func_00283B98@0038D3D8, func_00283B98@0038D3E8, func_00282978@0038D3F4,
   func_0029F338@0038D404]; data addresses materialised [0x3d560000]; float
   constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x30 ; sd $s0, 0x0($sp) ; swc1 $f21, 0x28($sp) ; daddu $s0, $a0, $zero ;
   swc1 $f20, 0x20($sp) ; sd $ra, 0x10($sp). Not matched in this wave: the
   body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
