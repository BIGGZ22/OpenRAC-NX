/* func_L02_00421DA0 - 1232 bytes, 2 place(s), level 02_maktar_nebula,
   retail at 0x00421da0. Screened with tools/w10_screen.py: 31 call(s)
   [func_00282CF0@00421E04, func_002A7878@00421E68, func_002A77E0@00421E74,
   func_002A7878@00421EA0, func_002ACEC0@00421EB4, func_002A7878@00421EC0,
   func_00282F20@00421ED4, func_002A7878@00421EE0, func_002A78D8@00421EFC,
   func_002A78D8@00421F14, func_002A78D8@00421F2C, 0x1163b0@00421F44,
   func_00282D30@00421F64, func_002C2210@00421F9C, func_00283CF0@00421FB8,
   func_00282CF0@00421FF8, func_00282F20@00422008, func_00282CC0@00422018,
   func_002C2210@00422050, func_00283CF0@0042206C, 0x1163b0@00422090,
   func_00282D30@004220B0, func_002C2210@004220E8, func_00283CF0@00422104,
   func_00282CF0@00422144, func_00282F20@00422154, func_00282CC0@00422164,
   func_002C2210@00422198, func_00283CF0@004221B4, func_002C2210@004221F4,
   func_00283CF0@00422210]; data addresses materialised [0x1c0000,
   0x3c880000, 0x3e190000, 0x3f330000, 0x3f860000, 0xbf860000, 0x3eb30000,
   0x3e7a0000, 0x40030000, 0x3fb70000, 0x3e990000, 0x3e570000, 0x3e330000];
   float constants [0x3f800000, 0x3fc00000, 0x40800000, 0x40800000,
   0x3fc00000, 0x40800000, 0x3e800000, 0x40800000, 0x40800000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7AA20000@0x0150, LQ/SQ 7E420000@0x0154]. Head: addiu
   $sp, $sp, -0xD0 ; sd $s0, 0x50($sp) ; sd $s5, 0x78($sp) ; daddu $s0, $a1,
   $zero ; sd $s6, 0x80($sp) ; daddu $s5, $a0, $zero. Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
