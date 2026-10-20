/* VU0 macro mode: no C spelling. 24 bytes, level 02 (12 places).
   Retail at 0x2F1FB8: `ldc2 $1,0x0($a1); ldc2 $2,0x0($a2); <0x4BE2086C, a COP2 macro op the
   disassembler prints as .word>; nop; jr $ra; sdc2 $1,0x0($a0)` - a vector pair loaded from two
   arguments, combined in VU0 and stored to the first. ASM_FUNC candidate (the row must be kept in
   assembly; C cannot emit `ldc2`/`sdc2` or the macro op). */
