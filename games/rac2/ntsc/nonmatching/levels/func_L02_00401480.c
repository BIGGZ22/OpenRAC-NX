/* from src/levels/shared/L02_003C43F0.c: not written this wave - no C can place it.
 * 176 bytes, 3 places. Rule: the word after the prologue is 0x700014A9 (opcode 0x1C), which
 * the disassembler prints as `.word # INVALID $zero,$zero,0x14A9`; the next word is
 * `sq $v0,0x0($sp)`, so the pair is a 16-byte load/store whose load half is not an `lq`
 * (0x1E) but this 0x1C form. Checked against the sibling rows of the same family
 * (func_L02_00332A80, func_L02_00332D28: MMI/LQ/SQ) - same class.
 * The rest of the row is ordinary: frame 0x30, the 16-byte block at $sp, f0 = -0.3f at $sp+0,
 * f1 = 1.0f at $sp+0xC, func_00283098($sp, $sp, A0+0xC0), func_00282CC0($sp, $sp, A0+0x10),
 * the 0xB5A test on A0+0xAA picking $gp+0x2738 or $gp+0x273C, then func_L00_002EC608 and
 * func_L00_002EC970 on the stack block. If the 0x1C word is ever placed by an assembly route,
 * the surrounding body is written out here.
 */
