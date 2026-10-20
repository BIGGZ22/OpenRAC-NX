/* func_L00_0041F570 - 684 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041f570. Screened with tools/w10_screen.py: 12 call(s)
   [func_L00_0032A088@0041F5AC, func_002FEE50@0041F5C0,
   func_0029F338@0041F5C8, func_00300190@0041F5FC, func_00283CE0@0041F71C,
   func_00283CE0@0041F728, func_00283CE0@0041F734, func_00283CF0@0041F73C,
   func_00283CE0@0041F748, func_00283CF0@0041F750,
   func_L00_0041F820@0041F7B0, func_L00_0041F820@0041F7E4]; data addresses
   materialised [0x3d4c0000, 0x3dcc0000, 0x200000]; float constants
   [0xc0e00000, 0x40e00000, 0xff0000, 0x40800000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 78A20000@0x00f8, LQ/SQ 7C620000@0x00fc, LQ/SQ
   79020000@0x011c, LQ/SQ 7C820000@0x0120]. Head: addiu $sp, $sp, -0x50 ; sd
   $s3, 0x18($sp) ; sd $s4, 0x20($sp) ; daddu $s3, $a0, $zero ; sd $s0,
   0x0($sp) ; sd $s2, 0x10($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
