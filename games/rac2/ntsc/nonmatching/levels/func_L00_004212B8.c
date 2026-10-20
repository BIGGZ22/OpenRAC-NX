/* func_L00_004212B8 - 1820 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x004212b8. Screened with tools/w10_screen.py: 49 call(s)
   [func_00282DB0@0042135C, func_00282D30@0042136C, func_003349E8@004213B4,
   func_00282DB0@004213E4, func_00282D30@004213F4, func_003349E8@0042143C,
   func_00283CE0@00421444, func_00283CE0@00421450, func_00283CE0@0042145C,
   func_00283CF0@00421464, func_00282DB0@00421488, func_00282D30@0042149C,
   func_003349E8@004214E4, func_00334710@00421510, func_00282DB0@00421530,
   func_00282D30@00421540, func_00282CF0@00421550, func_00282DB0@0042155C,
   func_00282D30@0042156C, func_00283CE0@004215B4, func_003348A8@00421644,
   func_00283CE0@00421688, func_00334730@004216E8, func_00282DB0@00421700,
   func_00282DF0@0042170C, func_00282DB0@00421738, func_00334730@0042174C,
   func_00282DB0@00421758, func_00282D30@00421768, func_00282CF0@00421778,
   func_00282D08@004217B4, func_00277258@004217D8, func_00282DB0@00421804,
   func_00282D30@00421818, func_00282CF0@00421828, func_00282978@00421830,
   func_00282D30@0042186C, func_00282CF0@0042187C, func_00283CE0@004218CC,
   func_00282DB0@004218FC, func_00282D30@00421910, func_00282CF0@00421920,
   func_00282DB0@00421934, func_00282D30@00421944, func_00282CF0@00421954,
   func_00282D08@00421968, func_00282DB0@0042197C, func_00282D30@0042198C,
   func_00282CF0@0042199C]; data addresses materialised [0x190000, 0x1c0000,
   0x1b0000, 0x2a0000, 0x3e4c0000, 0x3e4c0000]; float constants [0x3f000000,
   0x3f000000, 0x3f000000, 0x3f800000, 0x3f800000, 0x3f800000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7A220000@0x00c0, LQ/SQ 7C620000@0x00c4, LQ/SQ
   7A220000@0x0148, LQ/SQ 7C620000@0x014c, LQ/SQ 7A020000@0x01f0, LQ/SQ
   7C620000@0x01f4, LQ/SQ 7AA20000@0x02c0, LQ/SQ 7C620000@0x02c4, LQ/SQ
   7AE20000@0x02e0, LQ/SQ 7C620000@0x02e4, LQ/SQ 78620000@0x0474, LQ/SQ
   7EA20000@0x0478, COP1 46806320@0x04e8, LQ/SQ 7A620000@0x0530, LQ/SQ
   7EC20000@0x0534, LQ/SQ 7A620000@0x0540, LQ/SQ 7EA20000@0x0544, LQ/SQ
   7A620000@0x05a4, LQ/SQ 7E820000@0x05a8, LQ/SQ 7A820000@0x05ec, LQ/SQ
   7EC20000@0x05f0, LQ/SQ 7A620000@0x05f4, LQ/SQ 7E820000@0x05f8, LQ/SQ
   78620000@0x0638, LQ/SQ 7EA20000@0x063c]. Head: addiu $sp, $sp, -0x80 ;
   lui $v0, 0x19 ; sd $s0, 0x20($sp) ; addiu $a2, $v0, -0x61E0 ; sd $s2,
   0x30($sp) ; sd $s3, 0x38($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
