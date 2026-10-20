/* func_L18_002E42F8 - 1076 bytes, 3 place(s), level 18_damosel, retail at
   0x002e42f8. Screened with tools/w10_screen.py: 14 call(s)
   [func_00282C48@002E43D0, func_00282C48@002E43F8, func_00282C48@002E441C,
   func_00282C48@002E44E4, func_00282C48@002E4510,
   func_L18_002E0070@002E457C, func_L00_002B47A8@002E4610,
   func_002761F8@002E4628, func_00278E78@002E4638, func_002761F8@002E4678,
   func_L18_002E0070@002E46A8, func_L18_002E0070@002E46E0,
   func_L18_002E0070@002E46F0, func_L00_002B9B88@002E46FC]; data addresses
   materialised [0x190000, 0x3ecc0000, 0x3d8f0000, 0x3e4c0000, 0x3ee60000,
   0x3f4c0000, 0x3f4c0000, 0x3e8a0000, 0x3d8f0000, 0x3f330000, 0x3fa60000,
   0x1c0000, 0x3c230000]; float constants [0x40800000, 0x41800000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x60 ; lui $v0,
   0x19 ; sd $s0, 0x20($sp) ; addiu $v1, $zero, 0x74 ; sd $s3, 0x38($sp) ;
   addiu $s0, $v0, -0x61E0. Not matched in this wave: the body is straight-
   line code whose register allocation and the materialisation of its
   constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
