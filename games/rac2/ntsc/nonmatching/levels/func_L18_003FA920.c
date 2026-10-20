/* func_L18_003FA920 - 1324 bytes, 3 place(s), level 18_damosel, retail at
   0x003fa920. Screened with tools/w10_screen.py: 7 call(s)
   [0x115484@003FA968, func_L02_002F21C0@003FAC94,
   func_L02_004127F8@003FACAC, func_L02_004127F8@003FAD94,
   func_L18_003F8D28@003FADE8, func_L18_003F7818@003FADF8,
   func_L18_003FB500@003FAE04]; data addresses materialised [0x200000,
   0x1b0000, 0x1b0000, 0x200000, 0x1b0000, 0x1b0000, 0x200000, 0x200000,
   0x1b0000, 0x190000, 0x1b0000, 0x1b0000, 0x200000, 0x1b0000, 0x1b0000,
   0x1b0000]; float constants [0x41800000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [-]. Head: addiu $sp, $sp, -0x80 ; addiu $a2, $zero, 0x8 ; sd $s2,
   0x30($sp) ; sd $s7, 0x58($sp) ; daddu $s2, $a1, $zero ; sd $s0,
   0x20($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
