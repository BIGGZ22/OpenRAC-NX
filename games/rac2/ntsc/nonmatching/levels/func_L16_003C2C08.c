/* func_L16_003C2C08 - 1776 bytes, 2 place(s), level 16_snivelak, retail at
   0x003c2c08. Screened with tools/w10_screen.py: 40 call(s)
   [func_002A7750@003C2CE8, func_L16_003960E8@003C2D10,
   func_L14_004272D8@003C2D44, func_00282F20@003C2D6C,
   func_00282CC0@003C2D7C, func_002A8C28@003C2DC4, func_002E5CF8@003C2E4C,
   func_002E59F8@003C2E90, func_00288E18@003C2ED4,
   func_L00_002E6CF0@003C2F04, func_002A9640@003C2F2C,
   func_002E5CF8@003C2F40, func_002A7750@003C2F58, func_00282F20@003C2FA4,
   func_00282F20@003C2FC0, func_00282CC0@003C2FD8, func_002A8C28@003C3010,
   func_002E5CF8@003C3020, func_00282CF0@003C3038, func_00282F20@003C304C,
   func_0032DA58@003C3060, func_00282CC0@003C3070, func_002AB6D8@003C30C0,
   func_002B0A68@003C30F8, func_002A8508@003C3128, func_00288FB8@003C3144,
   0x115da8@003C315C, func_002A7750@003C316C, func_L00_0030DD70@003C3178,
   func_00288FB8@003C3188, func_L00_0030DD70@003C3194,
   func_002A7750@003C31A4, func_002E5CF8@003C31E0, func_00282948@003C31F0,
   func_00282948@003C3208, func_002A0430@003C3224, func_0029F338@003C322C,
   func_00283B98@003C326C, func_00283B98@003C32A0,
   func_L01_0031BC68@003C32BC]; data addresses materialised [0x2a0000,
   0x1a0000, 0x1a0000, 0x39980000, 0x1b0000, 0x40060000, 0x10000, 0x190000,
   0x1b0000, 0x1b0000, 0x3c0000, 0x1b0000, 0x10000, 0x190000, 0x1c0000,
   0x10000, 0x3dcc0000, 0x1c0000, 0x3e4c0000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x39980000, 0x1b0000, 0x39980000, 0x1b0000]; float constants [0x40400000,
   0x3f800000, 0x3f800000, 0x190000, 0x3f800000, 0x3f800000, 0xc0000000,
   0x40a00000, 0x40c00000, 0x40400000, 0x3f800000, 0x3f800000, 0x41a00000,
   0x40000000, 0x40800000, 0x41200000, 0x1b0000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 78620000@0x00ac, LQ/SQ 7C820000@0x00b0]. Head: addiu $sp,
   $sp, -0x110 ; sd $s3, 0xC8($sp) ; sd $s0, 0xB0($sp) ; daddu $s3, $a0,
   $zero ; sd $s1, 0xB8($sp) ; sd $s2, 0xC0($sp). Not matched in this wave:
   the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through
   a `mode(TI)` local, and this cc1 folds the destination offset into the
   `sq` where retail materialises the pointer first. What would unblock it:
   the exact source expression tree, so this cc1 picks the same registers in
   the same order. */
