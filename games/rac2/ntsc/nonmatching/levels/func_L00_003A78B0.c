/* func_L00_003A78B0 - 576 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a78b0. Screened with tools/w10_screen.py: 12 call(s)
   [func_00282D30@003A7958, func_00282CC0@003A7968, func_00282D30@003A7984,
   func_00282CC0@003A7998, func_00282D30@003A79B0, func_002A78D8@003A79C8,
   func_002A78D8@003A79E4, func_002A78D8@003A79FC, func_002A77E0@003A7A10,
   func_002C65A0@003A7A4C, func_002C65A0@003A7A84, func_00282CC0@003A7AAC];
   data addresses materialised [0xbdcc0000, 0x3e330000, 0x3dcc0000,
   0x48800000, 0x484d0000, 0x3e190000, 0xbd080000, 0x200000, 0x800000,
   0x1b0000, 0x200000, 0x1b0000]; float constants [-]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [-]. Head: addiu $sp, $sp, -0xA0 ; lui $at, 0xBDCC ; ori $at,
   $at, 0xCCCD ; mtc1 $at, $f12 ; sd $s0, 0x40($sp) ; sd $s1, 0x48($sp). Not
   matched in this wave: the body is straight-line code whose register
   allocation and the materialisation of its constant addresses have to be
   reproduced word for word. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
