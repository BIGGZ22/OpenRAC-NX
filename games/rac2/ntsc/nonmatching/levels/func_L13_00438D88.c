/* func_L13_00438D88 - 2704 bytes, 3 place(s), level 13_boldan, retail at
   0x00438d88. Screened with tools/w10_screen.py: 45 call(s)
   [func_L00_00341820@00438E6C, func_00282CF0@00438EEC,
   func_00282F20@00438F14, func_00282CC0@00438F38, func_002702C8@00438F54,
   func_002702C8@00438F74, func_002702C8@00438F94, func_00283CF0@004390D0,
   func_00283CF0@004390E0, func_00283CF0@00439168, func_00283CF0@00439204,
   func_L00_00341820@00439228, func_00282C48@00439270,
   func_002702C8@004392DC, func_002702C8@004392FC, func_002702C8@0043931C,
   func_L00_00341FA8@00439380, func_L00_00341820@004393C4,
   func_L00_00341820@00439410, func_L00_00341FA8@00439444,
   func_002702C8@00439494, func_002702C8@004394CC, func_002702C8@004394F4,
   func_L00_00341FA8@00439578, func_L00_00341820@004395AC,
   func_L00_00341820@004395C8, func_00282CF0@004395D8,
   func_00282CC0@004395E8, func_L00_00341820@00439620,
   func_00282F20@00439638, func_00282CC0@00439648, func_00282CF0@00439658,
   func_00282DF0@00439660, func_00282D30@00439690, func_00282DB0@0043969C,
   func_002831B0@004396A4, func_00282C48@004396C8, func_002703B0@0043974C,
   func_00282DD8@00439760, func_002ACEC0@00439774, func_00282F20@00439784,
   func_00282F20@00439794, func_00282DD8@004397A8, func_00282F20@004397B8,
   func_00282DD8@004397C8]; data addresses materialised [0x1b0000,
   0x3c230000, 0x190000, 0x200000, 0x3e330000, 0x200000, 0x1b0000, 0x190000,
   0x3c230000, 0x3c230000, 0x391d0000, 0x3e330000, 0x190000, 0x190000,
   0x3e330000, 0x3c230000, 0x3c230000, 0x3c230000, 0x190000, 0x200000,
   0x190000, 0x1a0000, 0x1c0000, 0x3fc90000, 0x391d0000, 0x3e330000,
   0x1c0000]; float constants [0x3f800000, 0x200000, 0x41a00000, 0x3f800000,
   0x40800000, 0x41a00000, 0x3f800000, 0x41a00000, 0x3f800000, 0x3f800000,
   0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7BA20000@0x00f0,
   LQ/SQ 7C620000@0x00f4, LQ/SQ 78820000@0x0648, LQ/SQ 7FA20000@0x064c].
   Head: addiu $sp, $sp, -0x160 ; lui $v1, 0x1B ; lw $v1, 0x22C4($v1) ; sd
   $s5, 0x108($sp) ; swc1 $f24, 0x150($sp) ; daddu $s5, $a0, $zero. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
