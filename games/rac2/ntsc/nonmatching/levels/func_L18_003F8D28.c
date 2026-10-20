/* func_L18_003F8D28 - 260 bytes, 3 place(s), level 18_damosel, retail at
   0x003f8d28. Screened with tools/w10_screen.py: 7 call(s)
   [func_002A7750@003F8D70, func_L18_003FB5E8@003F8D90,
   func_00282F20@003F8DA8, func_00282CC0@003F8DB8, func_002A89A8@003F8DCC,
   func_00283248@003F8DEC, func_002833F0@003F8DFC]; data addresses
   materialised [0xbe990000]; float constants [0x3f000000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [-]. Head: addiu $sp, $sp, -0x50 ; sd $s0, 0x20($sp) ; sd
   $s2, 0x30($sp) ; daddu $s0, $a2, $zero ; sd $s3, 0x38($sp) ; daddu $s2,
   $a0, $zero. Not matched in this wave: the body is straight-line code
   whose register allocation and the materialisation of its constant
   addresses have to be reproduced word for word. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
