/* func_L00_0040AFB8, 224 bytes, 27 placements.  NOT ATTEMPTED - screened out (TI128 class).
   Quadword traffic with no MMI zero: `lq $v1,0x10($s0)` at +0x88 and `sq $v1,0x10($sp)` at
   +0xB8 (words 0x7A030010 / 0x7FA30010), on top of two LIVE `mult` (0x00861818 at +0x28 and
   0x00461018 at +0x3C - opcode 0x00 funct 0x18, printed INVALID by rabbitizer but alive).
   That is the wave-6 TI128 copy wall: this cc1 folds a constant destination offset into the
   sq where retail materialises the pointer first, so the row waits for a compiler-side lever
   or an ASM_FUNC decision. */
