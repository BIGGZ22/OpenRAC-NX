/* func_L05_003CFB90 (148 bytes, 2 places) -- NEAR MISS, not integrated.
   state: everything matches except a rotation of three constant stores in the block
   setup. The block is filled with b19 = 2, i14 = 0x10000, i20 = 1, p10 = a0,
   h1A = *(u16 *)(a0 + 0xAA), b18 = 0, then func_00282CF0(&block, a0 + 0x10, a1),
   func_00282F20(&block, &block, 1.0f), f0C = 5627.9248046875f (0x45AFDF66),
   f1C = 5.0f, func_002A8E20(a0, &block): the three calls, the delay slots, the two
   swc1, the fp1A load and the epilogue are byte-identical.
   retail emits `sb $a1,0x19($sp)` (a1 = 2), `sw $v0,0x14($sp)` (v0 = 0x10000),
   `sw $v1,0x20($sp)` (v1 = 1); ours emits the same three stores rotated:
   `sw $a1,0x20` (a1 = 1), `sb $v0,0x19` (v0 = 2), `sw $v1,0x14` (v1 = 0x10000).
   Two source orders tried (b19/i14/i20 first, then p10, h1A, b18; and p10 before
   i14/i20) - the emitted rotation is identical, so cc1 normalises it.
   The float literal round-trips exactly (5627.9248046875f = 0x45AFDF66), which is
   worth reusing. */
