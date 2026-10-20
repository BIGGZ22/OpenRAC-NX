/* Jump table: no C can be placed. 136 bytes, level 00 (27 places).
   Retail at 0x2ADFE8: `addiu $v1,$a0,-0x1C; sltiu $v0,$v1,0x4C; beqz` to the zero return, then
   `lui $v0,0x2A; sll $v1,$v1,2; addiu $v0,$v0,-0x90; addu $v1,$v1,$v0; lw $a0,0x0($v1); jr $a0` at
   +0x24 - a 76-entry dispatch table at 0x0029FF70 tail-called through a register.
   The cases return a float constant (0xBF75BE0D, 0xBFA78D37) or 1, and one case loops while
   `0x19AA58 > 25.0f`. A C switch emits its own .rodata table and `jalr`, so the row needs either a
   hand-written assembly case or the table named as data. ASM_FUNC candidate. */
