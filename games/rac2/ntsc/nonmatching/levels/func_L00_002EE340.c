/* from src/levels/shared/L00_002E5918.c: no C attempt; VU0 macro mode. Retail, 72
 * bytes: ldc2 $1,0($a0); ldc2 $2,0($a1); then seven COP2 macro-mode words
 * (0x4BC000C3, 0x4BC2086C, 0x4A2210AA, 0x4BC1086A, 0x4B01083D, 0x4B01184A,
 * 0x4B020847 - vmul/vopm/vadd-family over the two loaded quads), 0x48240800
 * (qmfc2/cfc2 family, result to $a0), dsll32 $a0,$a0,0; nop; bgtz $a0 with
 * addi $v0,0x0 in the slot and addi $v0,0x1 on the fall-through; jr $ra; nop.
 * A plain C body cannot emit COP2 macro-mode ops: ASM_FUNC candidate, same class as
 * func_L00_002EE058/002EE078/002EE290 next door. */
