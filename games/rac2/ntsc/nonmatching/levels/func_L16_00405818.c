/* func_L16_00405818 - 2152 bytes, 2 place(s), level 16_snivelak, retail at
   0x00405818. Screened with tools/w10_screen.py: 67 call(s)
   [func_L11_00458D00@00405854, func_00281DE8@00405868,
   func_00282CF0@004059B4, func_00282F20@004059C8, func_00282CC0@004059D8,
   func_00282DD8@004059E8, func_00282F20@004059F8, func_00282F20@00405A0C,
   func_00282CC0@00405A1C, func_00282CC0@00405A2C, func_00282CF0@00405A3C,
   func_00282CC0@00405A4C, func_00282CF0@00405A5C, func_00280B90@00405A6C,
   func_00282CC0@00405AB0, func_00282CC0@00405AC0, func_00280B90@00405AD0,
   func_00282F20@00405AE4, func_00283180@00405B18, func_00283198@00405B24,
   func_00283B98@00405BD0, func_00281DE8@00405BE8, func_00282F20@00405C54,
   func_00282F20@00405C68, func_00282CC0@00405C78, func_00282CC0@00405C88,
   func_00282CF0@00405C98, func_00282CC0@00405CA8, func_00282CC0@00405CB8,
   func_00282CC0@00405CC8, func_00282CF0@00405CD8, func_00280B90@00405CE8,
   func_00282F20@00405D4C, func_00282F20@00405D5C, func_00282CC0@00405D6C,
   func_00282CC0@00405D7C, func_00282CF0@00405D8C, func_00282CC0@00405D9C,
   func_00282CC0@00405DAC, func_00282CC0@00405DBC, func_00282CF0@00405DCC,
   func_00280B90@00405DDC, func_00282F20@00405E40, func_00282F20@00405E50,
   func_00282CC0@00405E60, func_00282CC0@00405E70, func_00282CF0@00405E80,
   func_00282CC0@00405E90, func_00282CC0@00405EA0, func_00282CC0@00405EB0,
   func_00282CF0@00405EC0, func_00280B90@00405ED0, func_00282F20@00405F34,
   func_00282F20@00405F44, func_00282CC0@00405F54, func_00282CC0@00405F64,
   func_00282CF0@00405F74, func_00282F20@00405F8C, func_00282CC0@00405F9C,
   func_00282CC0@00405FAC, func_00282CC0@00405FBC, func_00282CF0@00405FCC,
   func_00280B90@00405FDC, func_002A7940@00405FE4, func_00281EB8@00406004,
   func_002A7940@0040601C, func_00281EB8@00406040]; data addresses
   materialised [0x3dcc0000, 0x1c0000, 0x1b0000, 0x800000, 0x3c8e0000,
   0x3da30000, 0x1b0000, 0xbf190000, 0x3ecc0000, 0x80800000]; float
   constants [0x3f800000, 0x40e00000, 0x40800000, 0xc0800000, 0x3f800000,
   0x3f000000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7AC20000@0x027c,
   LQ/SQ 7FA20000@0x0280, LQ/SQ 7AE20000@0x0288, LQ/SQ 7C620000@0x028c].
   Head: addiu $sp, $sp, -0x160 ; sd $s0, 0x100($sp) ; daddu $a0, $sp, $zero
   ; sd $s1, 0x108($sp) ; daddu $s0, $t1, $zero ; sd $s2, 0x110($sp). Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
