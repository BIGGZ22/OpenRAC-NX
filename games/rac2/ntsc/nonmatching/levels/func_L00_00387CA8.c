/* func_L00_00387CA8 - 580 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00387ca8. Screened with tools/w10_screen.py: 16 call(s)
   [func_0029EFD0@00387CDC, func_002833F0@00387D50, func_002FE8D8@00387D5C,
   func_00282F20@00387D70, func_00282DD8@00387D84, func_00282DB0@00387D90,
   func_00283198@00387DA8, func_00282D30@00387DB8, func_00283180@00387DC0,
   func_00283958@00387DD4, func_00283738@00387DE4, func_00282C88@00387E30,
   func_002FEC78@00387E6C, func_0029F338@00387E7C, func_002FED90@00387E9C,
   func_002A0F48@00387EB0]; data addresses materialised [0x190000, 0x140000,
   0x3cd60000]; float constants [0x3f400000, 0x3f800000, 0x3f000000,
   0x260000]; $gp-relative small data: no   saves: -; callee-saved registers
   saved [-]; words plain C cannot emit [LQ/SQ 7A220000@0x007c, LQ/SQ
   7E020000@0x0080, LQ/SQ 7A420000@0x0084, LQ/SQ 7EA20000@0x0088, LQ/SQ
   7A620000@0x0198, LQ/SQ 7C620000@0x019c, LQ/SQ 7A620000@0x01a4, LQ/SQ
   7C820000@0x01a8]. Head: addiu $sp, $sp, -0xC0 ; sd $s1, 0x78($sp) ; sd
   $s2, 0x80($sp) ; daddu $s1, $a0, $zero ; sd $s6, 0xA0($sp) ; daddu $s2,
   $a1, $zero. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
