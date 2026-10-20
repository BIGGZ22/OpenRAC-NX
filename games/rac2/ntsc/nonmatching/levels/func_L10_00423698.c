/* func_L10_00423698 - 228 bytes, 2 place(s), level 10_hrugis_cloud, retail
   at 0x00423698. Screened with tools/w10_screen.py: 9 call(s)
   [func_L05_0037B210@004236A4, func_L05_00379F18@004236AC,
   func_002ED8A0@004236E4, func_L05_00379F28@0042371C,
   func_L05_0037D6E8@00423724, func_L05_00379F68@0042372C,
   func_L05_00445A98@00423734, func_0029F338@00423748,
   func_0029F338@00423760]; data addresses materialised [0x220000,
   0x2a0000]; float constants [0x41000000]; $gp-relative small data: yes
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [-]. Head: addiu $sp, $sp, -0x10 ; sd $s0, 0x0($sp) ; sd $ra, 0x8($sp) ;
   jal func_37CFD8 ; lui $s0, 0x22 ; jal func_37BD00. Not matched in this
   wave: the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
