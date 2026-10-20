/* func_L00_0041EE58 - 1812 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041ee58. Screened with tools/w10_screen.py: 43 call(s)
   [func_002B4038@0041EEA0, func_0029F338@0041EEB0, func_002FEE50@0041EEB8,
   func_00300190@0041EECC, func_002AFFD8@0041EEE8,
   func_L00_00420310@0041EF10, func_00282CF0@0041EF34,
   func_002AB1F0@0041EF3C, func_00282D30@0041EF58, func_00282CC0@0041EF68,
   func_00282DF0@0041EF70, func_00282D30@0041EFA4, func_002FEE70@0041EFE0,
   func_00282CC0@0041EFF4, func_00282F20@0041F048, func_002761F8@0041F084,
   func_L00_0041FC20@0041F0B4, func_L00_0041FC20@0041F0CC,
   func_00334160@0041F0DC, func_002E5CF8@0041F0EC, func_00282948@0041F118,
   func_L00_004200F0@0041F128, func_002A77E0@0041F158,
   func_002C65A0@0041F198, func_002C65A0@0041F204, func_002C65A0@0041F270,
   func_00282CF0@0041F29C, func_00282D30@0041F2B4, func_002A77E0@0041F300,
   func_002A7878@0041F320, func_002A78D8@0041F330, func_002A78D8@0041F348,
   func_002A78D8@0041F360, func_002C65A0@0041F3A4, func_00282CC0@0041F3B4,
   func_002C65A0@0041F428, func_L00_002E6EF8@0041F450,
   func_00282948@0041F460, func_002FEE50@0041F470, func_0029F338@0041F478,
   func_L00_002E6EF8@0041F48C, func_002FEE50@0041F51C,
   func_0029F338@0041F524]; data addresses materialised [0x140000, 0x260000,
   0x190000, 0x45af0000, 0x1c0000, 0x48cd0000, 0x200000, 0x1b0000, 0x200000,
   0x200000, 0x1b0000, 0x200000, 0x48760000, 0xff0000, 0x1b0000, 0x200000,
   0x3ecc0000, 0x3dcc0000, 0x484d0000, 0x47240000, 0x3e4c0000, 0x3f190000,
   0x800000, 0x1b0000, 0x420000, 0x200000, 0x45030000, 0x48cd0000, 0x1b0000,
   0x200000, 0x420000, 0x420000, 0x447f0000]; float constants [0x10000,
   0x3f800000, 0x1b0000, 0x40000000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A020000@0x007c, LQ/SQ 7FA20000@0x0080, LQ/SQ 7A220000@0x01dc, LQ/SQ
   7E020000@0x01e0, LQ/SQ 78820000@0x0240, LQ/SQ 7E820000@0x0244, LQ/SQ
   7BA20000@0x0468, LQ/SQ 7C620000@0x046c]. Head: addiu $sp, $sp, -0x100 ;
   sd $s3, 0x98($sp) ; sd $s0, 0x80($sp) ; daddu $s3, $a0, $zero ; sd $s1,
   0x88($sp) ; sd $s4, 0xA0($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
