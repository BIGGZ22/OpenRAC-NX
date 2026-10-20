/* Not attempted in this run.
 *
 * src/core/00129450.c line 47. 592 bytes of Sony libmpeg (the unit that
 * rac1 covers with src/core/00125630.c, "macroblock/slice/picture decoder").
 * Ordinary C - the eleven `.word # INVALID` words are R5900 `mult rd,rs,rt`
 * (opcode 0x00, funct 0x18, rd set), exactly the trap the brief warns about,
 * not a reason to avoid the row.
 *
 * Read off the head so the next worker starts where this one stopped: the
 * frame is 0xB0 and s0 = arg0, s5 = arg3, fp = arg2; arg1 is divided by the
 * word at 0x12C($s0) (`div $zero,$a1,$v0` -> quotient $s7 = macroblock row,
 * remainder $s6 = column), and the two are scaled by 16 (`sll $s7,4`,
 * `sll $s6,4`). The body then waits on the IPU: it reads the halfword at
 * 0x1000D400, `srl 8` + `andi 1`, and spins while bit 0 is set, then
 * multiplies by 0x140 (`mult $a0,$v0`) - the picture width in the IPU's
 * 16-bit units. Pointers into the object are taken at 0x6B8/0x6C0/0x6C4/0x6C8
 * and the descriptor word comes from 0x810($s0).
 *
 * Nearest C already in the tree: src/core/0012E110.c's neighbours and the
 * matched libmpeg bodies func_00127378..func_00128860 in rac1
 * (src/core/00125630.c), which use the same 0x10002010 IPU_CTRL spin idiom.
 */
