/* func_L00_0038F178 - 604 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038f178. Screened with tools/w10_screen.py: 18 call(s)
   [func_00282D30@0038F218, func_002A7A00@0038F230, func_00282CC0@0038F240,
   func_00282C88@0038F248, func_002A7878@0038F258, func_002A7878@0038F270,
   func_002A7878@0038F280, func_00283D38@0038F298, func_002A7878@0038F2A8,
   func_00283D38@0038F2C0, func_002A7878@0038F2D0, func_00283CF0@0038F2D8,
   func_002A7878@0038F2E8, func_00283CF0@0038F2FC, func_002A7878@0038F30C,
   func_00283CF0@0038F320, func_002B00D8@0038F350, func_002BAA28@0038F380];
   data addresses materialised [0x1b0000, 0x3e190000, 0x3c880000,
   0x3d4c0000, 0x80400000, 0x80400000, 0x80100000, 0x3a910000, 0x10000];
   float constants [0x3f000000, 0x3f800000, 0xbf000000, 0x3e800000,
   0x80000000, 0x41f00000, 0x41200000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [COP1
   46800020@0x01b8]. Head: addiu $sp, $sp, -0xD0 ; lui $v0, 0x1B ; lw $v0,
   0x19C8($v0) ; sd $s0, 0x50($sp) ; sd $s1, 0x58($sp) ; andi $v0, $v0, 0x7.
   Not matched in this wave: the float code has to come out of the same
   expression tree to keep the FPU registers. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
