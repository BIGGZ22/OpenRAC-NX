/* VU0 macro mode: no C spelling. 32 bytes, level 02 (9 places).
   Retail at 0x2F21C0: `ldc2 $1,0x0($a0); ldc2 $2,0x0($a1);` then four COP2 macro ops the
   disassembler prints as .word (0x4B82086C, 0x4B81086A, 0x4B010841, 0x48240800), `jr $ra;
   mtc1 $a0,$f0` - a vector transform of the two arguments whose result is also copied into $f0.
   ASM_FUNC candidate: C cannot emit the ldc2/sdc2 loads nor the macro ops. */
