/* func_L00_00420C38 - 1660 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00420c38. Screened with tools/w10_screen.py: 15 call(s)
   [func_00283CE0@00420CDC, func_00283CE0@00420CF8, func_00283CE0@00420D64,
   func_00283CE0@00420D80, func_00283CE0@00420DD0, func_00283CE0@00420DEC,
   func_003349E8@00420E10, func_00283CE0@00420EC0, func_00283CE0@00421020,
   func_00283CE0@0042103C, func_003349E8@00421064, func_00283CE0@004211CC,
   func_00283CE0@004211F0, func_00282978@00421254, func_00283CE0@0042127C];
   data addresses materialised [0x190000, 0x190000, 0x190000, 0x190000,
   0x190000, 0x190000, 0x190000, 0x190000, 0x190000, 0x190000, 0x190000,
   0x190000, 0x190000, 0x190000]; float constants [0x3f800000, 0x3f800000,
   0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78820000@0x01e8, LQ/SQ 7C620000@0x01ec,
   LQ/SQ 78820000@0x043c, LQ/SQ 7C620000@0x0440, LQ/SQ 7A420000@0x05cc,
   LQ/SQ 7C620000@0x05d0]. Head: addiu $sp, $sp, -0x30 ; lw $v0, 0x2598($gp)
   ; sd $s0, 0x0($sp) ; sd $s2, 0x10($sp) ; sd $ra, 0x18($sp) ; daddu $s2,
   $a0, $zero. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
