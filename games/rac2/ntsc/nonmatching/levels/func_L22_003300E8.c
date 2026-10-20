/* func_L22_003300E8 (124 bytes, 2 places) -- MMI, not C.
   Four LQ/SQ words (0x7A420000/0x7C620000 copy the caller's doubleword into the sub
   object at s0+0xA0; 0x78820000/0x7FA20000 copy s1+0x10 onto the stack), then
   jal func_32F7A8(s1, s0, sp, sp+0x10, 0) and the `lw 0x94($s0)` / 8 answer.
   ASM_FUNC or the TI128 levers. */
