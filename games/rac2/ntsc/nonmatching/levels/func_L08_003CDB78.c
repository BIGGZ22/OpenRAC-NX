/* func_L08_003CDB78 - 1056 bytes, 3 place(s), level 08_tabora, retail at
   0x003cdb78. Screened with tools/w10_screen.py: 29 call(s)
   [func_00282F20@003CDBC8, func_00282CC0@003CDBDC, func_00282F20@003CDBF0,
   func_00282CC0@003CDC00, func_00277258@003CDC14, func_0029FCC0@003CDC30,
   func_002761F8@003CDC48, func_0029FCC0@003CDC64, func_002761F8@003CDC7C,
   func_002761F8@003CDCB4, func_00282E48@003CDCCC, func_00282E48@003CDCE4,
   func_00282CF0@003CDCFC, func_00282F20@003CDD0C, func_00282CC0@003CDD1C,
   func_002ABBB8@003CDD9C, func_002E5DA0@003CDDBC, func_00282CF0@003CDDD4,
   func_00282CC0@003CDDE4, func_00278E78@003CDE00,
   func_L08_002D2ED8@003CDE40, func_002A7470@003CDE5C,
   func_L08_003CDB38@003CDE84, func_002A7750@003CDE94,
   func_00283028@003CDEC0, func_00282D30@003CDED4,
   func_L00_002D2168@003CDF38, func_002A7750@003CDF48,
   func_00282CC0@003CDF58]; data addresses materialised [0x190000, 0x1c0000,
   0x1c0000, 0x810000, 0x1b0000, 0x3f4c0000, 0x1a0000, 0x1c0000, 0x1c0000,
   0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x39910000]; float constants
   [0x1c0000, 0x41100000, 0x41700000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7AC20000@0x0118, LQ/SQ 7E820000@0x011c, LQ/SQ 7A620000@0x0124, LQ/SQ
   7C620000@0x0128, LQ/SQ 78620000@0x0304, LQ/SQ 7E620000@0x0308, LQ/SQ
   78C20000@0x0334, LQ/SQ 7E620000@0x0338]. Head: addiu $sp, $sp, -0xA0 ;
   lwc1 $f12, 0x1004($gp) ; sd $s0, 0x40($sp) ; sd $s1, 0x48($sp) ; sd $s2,
   0x50($sp) ; lui $s1, 0x19. Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
