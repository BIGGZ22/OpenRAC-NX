/* func_L18_003FCD70 (180 bytes, 3 places) -- NEAR MISS, not integrated: no C can be
   placed.  The row opens on the real MMI word 0x700014A9 (rabbitizer: `$zero,
   $zero, 0x14A9`), the class COMPILER-NOTES keeps out of reach ("the 0x7000_xxxx
   family stays out of reach"), and it is followed by lq/sq pairs (0x7A420000,
   0x7FA20000) around a rand(5) call and a 0x90-strided loop over a stack quad.
   ASM_FUNC candidate.
   retail:
     003FCD70  addiu $sp, $sp, -0x40
     003FCD74  .word 0x700014A9          <- MMI
     003FCD78  sd    $s1, 0x28($sp)      ... prologue
     003FCD8C  .word 0x7FA20010          <- sq $v0,0x10($sp)
     003FCDB0  .word 0x7A420000          <- lq $v0,0($s2)
*/
