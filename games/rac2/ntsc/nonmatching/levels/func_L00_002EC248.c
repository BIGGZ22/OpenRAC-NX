/* func_L00_002EC248 - 956 bytes, 7 place(s), level 00_aranos_tutorial,
   retail at 0x002ec248. Screened with tools/w10_screen.py: 30 call(s)
   [func_002A7A00@002EC2FC, func_00282CC0@002EC30C,
   func_L00_00324F18@002EC324, func_002A7A00@002EC334,
   func_00282CC0@002EC344, func_00283098@002EC354, func_002A7878@002EC36C,
   func_00283CF0@002EC388, func_00283CF0@002EC39C, func_00283CF0@002EC3A8,
   func_002A7878@002EC3C8, func_00282C88@002EC3D8, func_00282CC0@002EC408,
   func_002A7878@002EC444, func_00283D38@002EC45C, func_002A7878@002EC46C,
   func_00283D38@002EC484, func_002A7878@002EC494, func_00283D38@002EC4AC,
   func_002A7878@002EC4BC, func_00283D38@002EC4D4, func_002A7878@002EC4E4,
   func_00283D38@002EC4FC, func_002A7878@002EC50C, func_00283D38@002EC524,
   func_002A7878@002EC534, func_00283D38@002EC54C, func_002A7878@002EC55C,
   func_00283D38@002EC570, func_002BAA28@002EC5A0]; data addresses
   materialised [0x39910000, 0x10000, 0x3dcc0000, 0x3ecc0000, 0x80100000,
   0x80100000, 0x100000, 0x100000, 0x80200000, 0x80400000, 0x200000,
   0x400000, 0x80100000, 0x80200000, 0x100000, 0x200000, 0x80000000,
   0x80400000, 0x400000]; float constants [0x3f800000, 0x3fc00000,
   0x3f000000, 0x41200000, 0x42200000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [COP1
   46800860@0x01b0]. Head: addiu $sp, $sp, -0xF0 ; lui $at, 0x3991 ; ori
   $at, $at, 0xA2B4 ; mtc1 $at, $f0 ; lui $v0, 0x1 ; sd $s3, 0x68($sp). Not
   matched in this wave: the float code has to come out of the same
   expression tree to keep the FPU registers. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
