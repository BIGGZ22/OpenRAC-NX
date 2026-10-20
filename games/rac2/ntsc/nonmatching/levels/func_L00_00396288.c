/* func_L00_00396288 - 1604 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00396288. Screened with tools/w10_screen.py: 25 call(s)
   [func_00291308@00396394, func_00283098@0039644C, func_0029FCC0@003964B4,
   func_00283098@003964DC, func_00282CC0@003964EC, func_0029FCC0@00396500,
   func_00282F20@00396510, func_00282CC0@00396520, func_0030CAD0@00396528,
   func_0030CCD0@00396530, func_L00_002E6EF8@00396540,
   func_00291308@003965B0, func_002E5CF8@003965E4, func_00282948@003965F4,
   func_002E59F8@00396634, func_L00_002AF568@00396648,
   func_0030DE88@00396684, func_L00_002AF568@00396694,
   func_002E5CF8@003966A8, func_L00_002AF7D0@003966B8,
   func_002E5CF8@00396700, func_L00_002AF7D0@00396740,
   func_00283098@00396784, func_00283CE0@00396804, func_00291450@00396874];
   data addresses materialised [0x190000, 0x140000, 0x3f4c0000, 0x3f660000,
   0x3f330000, 0x3d8f0000, 0x3f990000, 0x1c0000, 0x390000, 0x2a0000,
   0x190000, 0x190000, 0x1c0000, 0x3dcc0000, 0x3f330000]; float constants
   [0x260000, 0x40f00000, 0x41400000, 0x190000, 0x40000000, 0x40f00000];
   $gp-relative small data: yes   saves: -; callee-saved registers saved
   [-]; words plain C cannot emit [LQ/SQ 7A020000@0x0134, LQ/SQ
   7E420000@0x0138, LQ/SQ 7A420000@0x015c, LQ/SQ 7E220000@0x0160, LQ/SQ
   7FA00000@0x04d8]. Head: addiu $sp, $sp, -0x60 ; lui $v1, 0x19 ; sd $s3,
   0x28($sp) ; lui $v0, 0x14 ; sd $s6, 0x40($sp) ; addiu $a3, $v1, -0x61E0.
   Not matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain
   C reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
