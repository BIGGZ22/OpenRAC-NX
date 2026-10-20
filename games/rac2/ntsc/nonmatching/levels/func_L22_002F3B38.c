/* func_L22_002F3B38 (24 bytes, 2 places) -- COP2/VU0, needs ASM_FUNC, not C.
   `ldc2 $1,0x0($a1) ; ldc2 $2,0x0($a2) ; .word 0x4BC2086A (COP2 macro) ; nop ;
   jr $ra ; sdc2 $1,0x0($a0)`. The compiler never emits COP2 macro instructions
   (AGENTS.md, Handwritten assembly), so this is a vector routine for ASM_FUNC. */
