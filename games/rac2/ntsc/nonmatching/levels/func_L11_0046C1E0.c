/* func_L11_0046C1E0 - 1520 bytes, 2 place(s), level 11_joba, retail at
   0x0046c1e0. Screened with tools/w10_screen.py: 39 call(s)
   [func_0027C878@0046C238, func_0027C878@0046C290, func_00282CF0@0046C330,
   func_00282CF0@0046C344, func_00282DD8@0046C358, func_00282DF0@0046C364,
   func_00282D30@0046C3A0, func_00282CC0@0046C3B0, func_00282CF0@0046C3C0,
   func_00282CF0@0046C3FC, func_00282CF0@0046C40C, func_00282DD8@0046C41C,
   func_00282DF0@0046C424, func_00282D30@0046C458, func_00282CC0@0046C468,
   func_00282CF0@0046C478, func_00280B90@0046C4D4, func_00282CF0@0046C558,
   func_00282CF0@0046C568, func_00282DF0@0046C570, func_00282DF0@0046C57C,
   func_00282D30@0046C5C0, func_00282D30@0046C5FC, func_00282CC0@0046C610,
   func_00282CC0@0046C624, func_00282CF0@0046C634, func_00282D30@0046C644,
   func_00282CF0@0046C654, func_00282CC0@0046C664, func_00282CF0@0046C674,
   func_00282DD8@0046C684, func_00282DF0@0046C68C, func_00282D30@0046C6C0,
   func_00282CC0@0046C6D0, func_00282CF0@0046C6E0, func_00282CC0@0046C6F0,
   func_00282CF0@0046C700, func_00280B90@0046C758, func_002FC560@0046C788];
   data addresses materialised [0x1c0000, 0x1c0000, 0x1c0000, 0x50000];
   float constants [0x3f800000, 0x3f800000, 0x3f800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 7A420000@0x02fc, LQ/SQ 7FA20000@0x0300, LQ/SQ
   7A620000@0x0308, LQ/SQ 7C620000@0x030c, LQ/SQ 7A620000@0x03c4, LQ/SQ
   7E420000@0x03c8, LQ/SQ 7A420000@0x0400, LQ/SQ 7E620000@0x0404]. Head:
   addiu $sp, $sp, -0x260 ; sd $s7, 0x228($sp) ; daddu $s7, $a0, $zero ; sd
   $s0, 0x1F0($sp) ; sd $s1, 0x1F8($sp) ; daddu $s0, $a1, $zero. Not matched
   in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches
   it only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
