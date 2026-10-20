/* from src/levels/shared/L00_002E5918.c: no C attempt; VU0 macro mode plus MMI.
 * Retail, 72 bytes: ldc2 $1,0($a0); ori $v0,$0,0x7FFF; mfc1 $at,$f12; then
 * 0x70421488 twice (SPECIAL2/MMI), 0x48A11000 (COP2), 0x4BE20858 and 0x4BE1097C
 * (COP2 macro), addi $v1,$0,-0x8000 with 0x70631C88 twice (MMI), 0x48210800
 * (COP2), 0x702208E8 and 0x702308C8 (MMI) and the return delay word 0x700115C8
 * with jr $ra. The $v0 spin and $at/$v1 mask setup are MMI integer ops no plain C
 * reaches: ASM_FUNC candidate, same family as the COP2 neighbours. */
