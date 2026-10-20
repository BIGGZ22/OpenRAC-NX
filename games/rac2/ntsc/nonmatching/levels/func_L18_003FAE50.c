/* func_L18_003FAE50 - 1208 bytes, 3 place(s), level 18_damosel, retail at
   0x003fae50. Screened with tools/w10_screen.py: 26 call(s)
   [func_00282CF0@003FAED8, func_00282D30@003FAEEC, func_00282CC0@003FAEFC,
   func_002A7878@003FAF0C, func_00282CF0@003FAF8C, func_00282CF0@003FAFA0,
   func_00282CF0@003FAFB4, func_00282DD8@003FAFC4, func_00282DD8@003FAFD4,
   func_00282F20@003FAFE4, func_00282F20@003FAFF4, func_00281DE8@003FB008,
   func_00282CF0@003FB0BC, func_00282CC0@003FB0D4, func_00282CF0@003FB0EC,
   func_00282CC0@003FB104, func_00280B90@003FB11C, func_00282F20@003FB12C,
   func_00282F20@003FB13C, func_00282CF0@003FB14C, func_00282CC0@003FB164,
   func_00282CF0@003FB17C, func_00282CC0@003FB194, func_00281DE8@003FB1B0,
   func_00280B90@003FB218, func_00282CC0@003FB230]; data addresses
   materialised [0x2a0000, 0x2a0000, 0x1b0000, 0xff0000, 0xff0000, 0x1c0000,
   0x1c0000, 0x1b0000, 0x2a0000]; float constants [0x3f800000, 0x3f800000,
   0x40000000, 0xbf800000, 0x40000000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78620000@0x0064, LQ/SQ 7FA20000@0x0068, COP1 468018E0@0x00d8, COP1
   46800860@0x00ec, COP1 46000064@0x00fc, COP1 46800860@0x01ec, LQ/SQ
   7BA200F0@0x0274, LQ/SQ 7FA20060@0x0288, LQ/SQ 7BA200F0@0x028c, LQ/SQ
   7FA20070@0x02a0, LQ/SQ 7BA200F0@0x02a4, LQ/SQ 7FA20080@0x02b8, LQ/SQ
   7BA200F0@0x02bc, LQ/SQ 7FA20090@0x02d0, LQ/SQ 7BA200F0@0x0304, LQ/SQ
   7FA20060@0x0318, LQ/SQ 7BA200F0@0x031c, LQ/SQ 7FA20070@0x0330, LQ/SQ
   7BA200F0@0x0334, LQ/SQ 7FA20080@0x0348, LQ/SQ 7BA200F0@0x034c, LQ/SQ
   7FA20090@0x035c, COP1 46800860@0x039c, LQ/SQ 7A820000@0x03d0, LQ/SQ
   7FA20000@0x03d4, COP1 46800860@0x03f0]. Head: addiu $sp, $sp, -0x180 ; sd
   $s0, 0x120($sp) ; addiu $v0, $sp, 0x50 ; swc1 $f20, 0x170($sp) ; addiu
   $v1, $sp, 0x10 ; sd $s1, 0x128($sp). Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
