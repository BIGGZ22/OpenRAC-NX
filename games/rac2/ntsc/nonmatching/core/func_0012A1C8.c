/* from src/core/00129450.c: not attempted in this run; ordinary libmpeg per-channel
   routine - the `.word` words are live R5900 multiplies, not a .rodata jump table */
/*
 * 0x12A1C8..0x12A3DC, 540 bytes. Real body: no `jr $ra` - it leaves through three sibling
 * `j` tails, func_12B138 twice and func_12B0D8 once.
 *
 * Profile: `addiu $sp,$sp,-0xB0` with `sd $s4,0x50` / `sd $ra,0xA0` / `sd $fp,0x90` /
 * `sd $s7,0x80` / `sd $s6,0x70` / `sd $s5,0x60` / `sd $s3,0x40` / `sd $s2,0x30` /
 * `sd $s1,0x20` / `sd $s0,0x10` - 16-byte slots. Note `$fp` is saved, so this unit is not
 * the `-mastra-quad-saves` region.
 *
 * The six `.word` instructions are NOT a jump table and the two-minute triage does not
 * apply: there is no `lui $vX,0x14` + `lw`/`addu` + `jr $t` sequence anywhere. They are
 * live multiplies rabbitizer will not print -
 *     0x00A22818 and 0x00518818 and 0x00622018 and 0x00622818  opcode 0x00 funct 0x18,
 *         the R5900's three-operand `mult rd,rs,rt` (low 64 bits into rd);
 *     0x72B39818                                               opcode 0x1C funct 0x18,
 *         MULT1, which rabbitizer prints as `INVALID`.
 * The stride is 0x140 (320): `addiu $v0,$zero,0x140` then `mult $a1,$v0`, and the same
 * multiply reappears at +260 and +316.
 *
 * The indirect calls are `jalr` through pointers held in the structure, not through a
 * table next to the code: `lw $v1,0x0($v0) ; jalr $v1` with `$v0 = $s7 + $s0` and
 * `$v0 = $s6 + $s0`. Fields in play: 0x590, 0x594, 0x5B8, 0x5C8, 0x6B8, 0x6BC, 0x6C0,
 * 0x6C8, 0x6CC, 0x81C, indexed by an argument-scaled 0x140 offset.
 *
 * Ordinary C (function-pointer calls plus 64-bit multiplies); the only cost is reading
 * the two loops and the three exits. func_130288 is called with the 0x14xxxx-0x4580
 * format string, so the exit paths are a small switch over 0x6CC / 0x81C.
 */
