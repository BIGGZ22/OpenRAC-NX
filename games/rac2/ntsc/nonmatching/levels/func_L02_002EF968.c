/* 128-bit block moves: no C spelling. 148 bytes, level 02 (8 places).
   Retail at 0x2EF968 saves five registers, calls func_2EF8C8(dst+0x40, ..., count 0x80) once, then
   runs a 4-iteration loop that per pass copies two floats with `lwc1`/`swc1` (from a source pointer
   in $s2), stores one word, and moves 16 bytes with `lq $s1,0x0($s0)` / `sq ...` (raw words
   0x7A220000 and 0x7E020000 at +0x54, opcodes 0x1E/0x1F, printed as .word by the disassembler).
   The argument registers are the ABI's 5th/6th slots ($t0/$t1 taken into $s1 and $a2 before the
   call). A struct copy in C does not select lq/sq here, so the row needs assembly. ASM_FUNC
   candidate. */
