/* func_L18_003F53D8 - 632 bytes, 3 place(s), level 18_damosel, retail at
   0x003f53d8. Screened with tools/w10_screen.py: 12 call(s)
   [func_002A9640@003F53F8, func_002A7750@003F544C, func_002B0ED8@003F5464,
   func_L18_003F7AB8@003F54BC, func_L18_003F74B8@003F551C,
   func_L18_003F74B8@003F5538, func_002A6C58@003F5568,
   func_L18_0033B628@003F55B0, func_L18_003F74B8@003F55C4,
   func_002B3BA8@003F55E8, func_L18_003F8E30@003F55FC,
   func_002A7750@003F5630]; data addresses materialised [0x10000, 0x190000];
   float constants [0x42b40000, 0xbf800000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [-]. Head: addiu $sp, $sp, -0x20 ; lui $a1, 0x1 ; sd $s0, 0x0($sp) ;
   addiu $a2, $zero, 0x1 ; sd $s2, 0x10($sp) ; daddu $s0, $a0, $zero. Not
   matched in this wave: the body is straight-line code whose register
   allocation and the materialisation of its constant addresses have to be
   reproduced word for word. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
