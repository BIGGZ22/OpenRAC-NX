/* func_L00_00384248 - 1072 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00384248. Screened with tools/w10_screen.py: 18 call(s)
   [func_00283098@003842C0, func_00282CC0@003842D4, func_002AEC18@00384300,
   func_002A05A8@00384308, func_002A0F48@00384314, func_002AA2E8@003843F8,
   func_002AA2E8@0038440C, func_00282948@0038447C, func_00283CE0@003844B0,
   func_00283CE0@003844BC, func_00283198@003844E4, func_00283CF0@003844F4,
   func_00282948@0038450C, func_00283098@0038459C, func_00282CC0@003845B0,
   func_00282F20@003845D4, func_L00_00384CA8@003845E8,
   func_002E5CF8@003845F8]; data addresses materialised [0x1c0000,
   0x3c880000, 0x39910000, 0x140000, 0x140000, 0x140000, 0x140000,
   0x3eb30000, 0xbee60000, 0x1b0000, 0x40c90000, 0x40490000, 0x140000,
   0x140000, 0x3d4c0000, 0x3c880000]; float constants [0x41200000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78620000@0x031c, LQ/SQ 7FA20000@0x0320].
   Head: addiu $sp, $sp, -0xA0 ; lui $v0, 0x1C ; sd $s0, 0x30($sp) ; sd $s1,
   0x38($sp) ; sd $s2, 0x40($sp) ; sd $s3, 0x48($sp). Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
