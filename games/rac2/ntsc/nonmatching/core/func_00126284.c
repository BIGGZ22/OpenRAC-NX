/* Not a C target as it stands. The body carries an MMI instruction
   (`0x700014A9`, opcode 0x1C, which rabbitizer prints as `.word # INVALID`)
   and a `sync` before the return, and its first words read `$t6`
   (`pref 0xD,-0x3233($t6)`) before anything writes it - the three tells the
   campaign treats as hand-written assembly. What is legible is a fixed-point
   packer: the six 16-bit arguments are sign-extended, `$a3` (`$t5` after the
   shift) selects one of 0x3B (59) cases through the jump table at 0x13B3F0,
   each case is a 64-bit shift of `mult $t2, $t0` (R5900 three-operand mult),
   and the tail writes twelve quadwords to the caller's structure at $t6 -
   offsets 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50
   with the literal tags 0x50, 0x51, 0x52, 0x53 - then `sync`. Belongs as
   ASM_FUNC, like func_0011B0B0. */
