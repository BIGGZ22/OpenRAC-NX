/* func_L00_0041B840 - 680 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041b840. Screened with tools/w10_screen.py: 9 call(s)
   [func_L00_0041AF60@0041B914, func_L00_0041AF60@0041B920,
   func_002A7878@0041B93C, func_002A7878@0041B980, func_002A7878@0041B9BC,
   func_002A7878@0041B9D4, func_002A7878@0041BA08, func_002A7878@0041BA24,
   0x1163b0@0041BA30]; data addresses materialised [0x3ecc0000, 0xbdcc0000,
   0x3dcc0000, 0x3e990000, 0x3f660000, 0xbca30000, 0xbc230000, 0xc0ff0000];
   float constants [0x3f800000, 0x3f000000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 78620000@0x0240, LQ/SQ 7C820000@0x0244, LQ/SQ 78820000@0x024c,
   LQ/SQ 7E820000@0x0250]. Head: addiu $sp, $sp, -0x80 ; lw $t0, 0x2460($gp)
   ; sd $fp, 0x50($sp) ; daddu $a0, $zero, $zero ; sd $s0, 0x10($sp) ; daddu
   $fp, $a1, $zero. Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
