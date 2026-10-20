/* func_L00_0041F820 - 1020 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041f820. Screened with tools/w10_screen.py: 30 call(s)
   [func_00281DE8@0041F8A4, func_00281DE8@0041F900, func_00282CF0@0041F944,
   func_00282CF0@0041F958, func_00282DD8@0041F96C, func_00282F20@0041F980,
   func_00282CC0@0041F990, func_00282CF0@0041F9A0, func_00282D30@0041F9B0,
   func_00282CC0@0041F9C0, func_00282CF0@0041F9D0, func_00282CF0@0041FA24,
   func_00282CF0@0041FA34, func_00282DD8@0041FA44, func_00282F20@0041FA54,
   func_00282CC0@0041FA64, func_00282CF0@0041FA74, func_00282D30@0041FA84,
   func_00282CC0@0041FA94, func_00282CF0@0041FAA4, func_00283CE0@0041FAAC,
   func_00283CE0@0041FAB8, func_00283CE0@0041FAD0, func_00283CF0@0041FAD8,
   func_00283D38@0041FAF0, func_00283CE0@0041FB08, func_00283CF0@0041FB10,
   func_00283D38@0041FB28, func_00280B90@0041FB48, func_00280B90@0041FB58];
   data addresses materialised [0x1c0000, 0xff0000, 0x1c0000, 0x100000,
   0x100000]; float constants [0x3f800000, 0x3f800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 78620000@0x0344, LQ/SQ 7FA20000@0x0348, LQ/SQ
   78E20000@0x0354, LQ/SQ 7C820000@0x0358, LQ/SQ 7BC20000@0x0360, LQ/SQ
   7D020000@0x0364, LQ/SQ 7A820000@0x036c, LQ/SQ 7C620000@0x0370]. Head:
   addiu $sp, $sp, -0x200 ; sd $s1, 0x188($sp) ; sd $s6, 0x1B0($sp) ; daddu
   $s1, $a0, $zero ; sd $s7, 0x1B8($sp) ; daddu $s6, $t1, $zero. Not matched
   in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches
   it only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
