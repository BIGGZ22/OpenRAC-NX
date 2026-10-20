/* Jump table: no C can be placed. 184 bytes, level 00 (27 places).
   Retail at 0x421A98 dispatches through `lw $a0,...; jr $a0` at +0x4c (the table is loaded with a
   `lui`+`addiu` pair before it). Same class as func_L00_002ADFE8 in this lot: a C switch emits its
   own .rodata table and `jalr` instead of the retail table + register tail call.
   ASM_FUNC candidate. */
