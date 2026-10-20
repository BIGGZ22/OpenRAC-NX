/* func_L07_002B3580 - 2356 bytes, 2 place(s), level 07_siberius, retail at
   0x002b3580. Screened with tools/w10_screen.py: 36 call(s)
   [func_00282C88@002B35F8, func_L00_002ADF80@002B3658,
   func_L00_002B47A8@002B3680, func_002761F8@002B369C,
   func_L00_002E2048@002B36AC, func_00278E78@002B36F4,
   func_002A90F8@002B3718, func_002761F8@002B3760,
   func_L00_002E2048@002B3770, func_002761F8@002B37C4,
   func_002761F8@002B3814, func_002ABB78@002B3850, func_002AB1F0@002B3864,
   func_002AD0B0@002B388C, func_002B0360@002B38F0, func_00282E48@002B3908,
   0x1290e0@002B3924, func_L00_002D3EA8@002B392C, func_00283248@002B393C,
   func_00282CF0@002B3954, func_00282DB0@002B3960, func_00282C88@002B3A50,
   func_002AFF60@002B3AB8, func_002AA2E8@002B3AEC, func_00282D30@002B3B1C,
   func_00282CF0@002B3B58, func_00282F20@002B3B68,
   func_L00_002B8E48@002B3BFC, func_L00_002D3638@002B3D18,
   func_L00_002D3638@002B3D38, func_002761F8@002B3D50,
   func_002B0360@002B3D6C, func_002B0360@002B3D78, func_00282C48@002B3DA8,
   0x1290e0@002B3DD8, func_L00_002B8E48@002B3DE4]; data addresses
   materialised [0x190000, 0x400c0000, 0x40390000, 0x190000, 0x1c0000,
   0x1a0000, 0x190000, 0x1c0000, 0x190000, 0x190000, 0x190000, 0x190000,
   0x3c230000, 0x190000, 0x3c230000, 0x190000, 0x190000, 0x190000, 0x1b0000,
   0x3ca30000, 0x190000, 0x3f5f0000, 0x190000, 0x190000, 0x190000, 0x190000,
   0x1b0000, 0x190000, 0x190000, 0x190000, 0x190000, 0xbf490000, 0x3f490000,
   0xbf490000, 0x190000, 0x3f490000, 0x190000, 0x190000, 0x3e4c0000,
   0xbf490000, 0x3f490000, 0x190000, 0x3fc90000, 0x190000, 0x3ee60000,
   0xbee60000, 0x1c0000, 0x1c0000, 0x3ecc0000, 0x1c0000, 0x190000]; float
   constants [0x3f800000, 0x42000000, 0xc2400000, 0xbf800000, 0x190000,
   0x3f800000, 0xbf800000, 0x190000, 0xbf800000, 0x3f800000, 0x3e800000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78820000@0x0350, LQ/SQ 7E820000@0x0354,
   LQ/SQ 78820000@0x04b0, LQ/SQ 7CA20000@0x04b4, LQ/SQ 78620000@0x0578,
   LQ/SQ 7E020000@0x057c, LQ/SQ 78620000@0x0588, LQ/SQ 7E020000@0x058c].
   Head: addiu $sp, $sp, -0xE0 ; lui $v0, 0x19 ; sd $s1, 0x78($sp) ; sd $s0,
   0x70($sp) ; addiu $s1, $v0, -0x61E0 ; sd $s2, 0x80($sp). Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
