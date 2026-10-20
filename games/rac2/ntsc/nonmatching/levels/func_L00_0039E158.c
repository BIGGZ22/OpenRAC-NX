/* func_L00_0039E158 - 940 bytes, 2 place(s), level 00_aranos_tutorial,
   retail at 0x0039e158. Screened with tools/w10_screen.py: 14 call(s)
   [func_00282E48@0039E214, func_00282E48@0039E24C,
   func_L00_00341FA8@0039E2FC, func_00282E48@0039E308,
   func_L00_00341820@0039E3B8, func_00282CF0@0039E3E8,
   func_00282CF0@0039E400, func_00282E20@0039E408, func_00283248@0039E414,
   func_00283248@0039E424, func_002AA2E8@0039E444, func_00282DF0@0039E44C,
   func_00282F20@0039E478, func_00282D30@0039E4B8]; data addresses
   materialised [0x190000, 0x200000, 0x190000, 0x461c0000, 0x44790000,
   0x190000, 0x3ca30000, 0x406c0000, 0x3f330000, 0x3b6c0000, 0x3a830000,
   0x3f7d0000]; float constants [0x200000, 0x40a00000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 7A420000@0x01cc, LQ/SQ 7FA20000@0x01d0, COP1
   46800860@0x0224, LQ/SQ 7A820000@0x0330, LQ/SQ 7C820000@0x0334, LQ/SQ
   7A020000@0x036c, LQ/SQ 7C620000@0x0370]. Head: addiu $sp, $sp, -0xB0 ; sd
   $s7, 0x88($sp) ; sd $s0, 0x50($sp) ; daddu $s7, $a0, $zero ; sd $s1,
   0x58($sp) ; sd $s2, 0x60($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
