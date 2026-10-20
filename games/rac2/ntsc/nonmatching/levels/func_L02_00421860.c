/* func_L02_00421860 - 1340 bytes, 2 place(s), level 02_maktar_nebula,
   retail at 0x00421860. Screened with tools/w10_screen.py: 33 call(s)
   [func_002AFFD8@004218B4, func_00282CC0@004218DC, func_00282F20@00421914,
   func_002761F8@0042194C, func_002E5CF8@00421974,
   func_L02_00421DA0@00421990, func_L02_00421DA0@004219AC,
   func_L02_00422270@004219C0, func_002A8BD0@004219DC,
   func_00277F48@00421A18, func_00282948@00421A28, func_002A7878@00421A48,
   func_002A77E0@00421A5C, func_002A7878@00421AA4, func_002A7878@00421AB8,
   func_002A7878@00421AC8, func_00282F20@00421AE4, func_002A7878@00421AFC,
   func_002C2210@00421B2C, func_002C2210@00421B58, func_00283CE0@00421B80,
   func_00283CE0@00421B8C, 0x1163b0@00421B98, func_002A77E0@00421BC4,
   func_002A7878@00421BEC, func_002A7878@00421BFC, func_002A7878@00421C0C,
   func_002A7878@00421C1C, func_00282F20@00421C3C, func_002C2210@00421C74,
   func_002C2210@00421CB8, func_0029F338@00421D48,
   func_L02_00422538@00421D58]; data addresses materialised [0x10000,
   0x45af0000, 0x1c0000, 0x1c0000, 0x1c0000, 0x810000, 0x3f4c0000,
   0x3eb30000, 0x3e7a0000, 0x3f330000, 0x3ccc0000, 0x3dcc0000, 0x3ccc0000,
   0x3d4c0000, 0x3b650000, 0x3d0f0000, 0x447f0000]; float constants
   [0x3f800000, 0xbf800000, 0x3f800000, 0xbf800000, 0x3f800000, 0x40000000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 7A420000@0x0044, LQ/SQ 7FA20000@0x0048,
   LQ/SQ 7A420000@0x0060, LQ/SQ 7EA20000@0x0064, LQ/SQ 7A220000@0x00a4,
   LQ/SQ 7C820000@0x00a8]. Head: addiu $sp, $sp, -0x100 ; sd $s2, 0xA0($sp)
   ; sd $s3, 0xA8($sp) ; sd $s0, 0x90($sp) ; daddu $s3, $a0, $zero ; sd $s1,
   0x98($sp). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
