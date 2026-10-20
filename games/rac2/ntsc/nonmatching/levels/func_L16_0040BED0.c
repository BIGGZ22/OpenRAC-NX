/* func_L16_0040BED0 - 1824 bytes, 2 place(s), level 16_snivelak, retail at
   0x0040bed0. Screened with tools/w10_screen.py: 26 call(s)
   [func_0029F338@0040BFA0, func_00283248@0040BFCC, func_002A7750@0040BFFC,
   func_L16_0040C7E8@0040C024, func_002E5CF8@0040C0AC,
   func_00282E80@0040C0DC, func_00282C10@0040C114, func_00282CF0@0040C128,
   func_00282F70@0040C13C, func_00282F70@0040C14C, func_00282E20@0040C1CC,
   func_00282E80@0040C1DC, func_00282CF0@0040C220, func_00282DB0@0040C230,
   func_00282F70@0040C260, func_00282F70@0040C278, func_00283248@0040C2A0,
   func_L00_002D70A0@0040C2D0, func_L00_002D7140@0040C350,
   func_002A7750@0040C364, func_002A7750@0040C3FC,
   func_L16_0040C5F0@0040C428, func_L02_002F21C0@0040C44C,
   func_00288E18@0040C4A0, func_002E5CF8@0040C56C, func_002E59F8@0040C5B4];
   data addresses materialised [0x1b0000, 0x1a0000, 0x190000, 0x1a0000,
   0x1a0000, 0x3e990000, 0x39910000, 0x1a0000, 0x1a0000, 0x190000, 0x190000,
   0x3c8e0000, 0x1a0000, 0x190000, 0x190000, 0x1a0000, 0x80000000, 0x190000,
   0x190000]; float constants [0x40200000, 0x42140000, 0x3f000000,
   0x3f000000, 0x1a0000, 0x1b0000, 0x41100000, 0x190000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 7A220000@0x03b4, LQ/SQ 7C620000@0x03b8, LQ/SQ
   78620000@0x0424, LQ/SQ 7C820000@0x0428]. Head: addiu $sp, $sp, -0x60 ;
   lui $v0, 0x1B ; lw $v0, -0x7100($v0) ; sd $s2, 0x20($sp) ; sd $s0,
   0x10($sp) ; daddu $s2, $a0, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
