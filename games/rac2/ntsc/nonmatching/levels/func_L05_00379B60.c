/* func_L05_00379B60 - 260 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x00379b60. Screened with tools/w10_screen.py: 4 call(s)
   [func_L05_00379968@00379B8C, func_002A7820@00379BBC,
   func_L05_0037B3A0@00379BF4, func_L00_002E6EF8@00379C3C]; data addresses
   materialised [0x1b0000, 0x220000, 0x220000, 0x380000]; float constants
   [-]; $gp-relative small data: yes   saves: -; callee-saved registers
   saved [-]; words plain C cannot emit [-]. Head: lui $v0, 0x1B ; lw $v0,
   0x2048($v0) ; addiu $sp, $sp, -0x30 ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp)
   ; slti $v0, $v0, 0xA. Not matched in this wave: the body is straight-line
   code whose register allocation and the materialisation of its constant
   addresses have to be reproduced word for word. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
