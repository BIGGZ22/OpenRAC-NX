/* func_L08_003CC4B8 - 664 bytes, 3 place(s), level 08_tabora, retail at
   0x003cc4b8. Screened with tools/w10_screen.py: 19 call(s)
   [func_0029FCC0@003CC4F8, func_002A77E0@003CC500, func_002A7A00@003CC540,
   func_002A77E0@003CC54C, func_002C3150@003CC56C, func_002A7A00@003CC590,
   func_00282CC0@003CC5A0, func_002A77E0@003CC5AC, func_002C3150@003CC5CC,
   func_00282CF0@003CC5E8, func_00282F20@003CC614, func_00282F20@003CC638,
   func_00282CC0@003CC650, func_00283CE0@003CC66C, func_00282C48@003CC678,
   func_00283D38@003CC688, func_00283CE0@003CC6BC, func_002C2FD8@003CC6F4,
   func_00282CC0@003CC704]; data addresses materialised [0x1c0000,
   0x3c880000, 0x3b800000, 0xbe990000, 0x3dcc0000, 0x1b0000]; float
   constants [0x3f000000, 0x40400000, 0x1b0000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [-]. Head: addiu $sp, $sp, -0xC0 ; daddu $a1, $zero, $zero ; sd $s7,
   0x88($sp) ; daddu $a2, $sp, $zero ; daddu $s7, $a0, $zero ; sd $s2,
   0x60($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
