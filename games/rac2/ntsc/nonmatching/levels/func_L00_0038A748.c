/* func_L00_0038A748 - 544 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038a748. Screened with tools/w10_screen.py: 16 call(s)
   [func_0029EFD0@0038A784, func_002833F0@0038A7E4, func_002FE8D8@0038A7F0,
   func_00282F20@0038A804, func_00282DD8@0038A818, func_00282DB0@0038A824,
   func_00283198@0038A83C, func_00282D30@0038A84C, func_00283180@0038A854,
   func_00283958@0038A868, func_00283738@0038A878, func_00282C88@0038A8A4,
   func_002FEC78@0038A8E4, func_0029F338@0038A8F4, func_002FED90@0038A914,
   func_002A0F48@0038A928]; data addresses materialised [0x190000,
   0x3cd60000]; float constants [0x3f800000, 0x3f000000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 7A220000@0x0070, LQ/SQ 7E020000@0x0074, LQ/SQ
   7A420000@0x0078, LQ/SQ 7EA20000@0x007c, LQ/SQ 7A620000@0x016c, LQ/SQ
   7C620000@0x0170, LQ/SQ 7A620000@0x0178, LQ/SQ 7C820000@0x017c]. Head:
   addiu $sp, $sp, -0xD0 ; sd $s7, 0xA8($sp) ; sd $s1, 0x78($sp) ; daddu
   $s7, $a0, $zero ; sd $s2, 0x80($sp) ; daddu $s1, $a1, $zero. Not matched
   in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches
   it only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
