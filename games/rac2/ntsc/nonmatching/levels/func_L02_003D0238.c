/* 0 of 1 places match - 128-bit LQ/SQ, three pairs (out of C reach).
   Level 2 (02_maktar_nebula), 0x003D0238, 184 bytes: a spawn/initialise body
   (zero a field, `func_32ADC0` with the float at s0+0xD8, `func_31E6A0`, the
   0xFFFE/0x1000 flag raise at self+0x34, then `func_2EF388`). Three 16-byte block
   moves block it:
     +0x4c `.word 0x78A20000` (LQ) / +0x50 `.word 0x7C820000` (SQ)   a1 = s0+0xA0 -> a0 = self+0x10
     +0x58 `.word 0x78A20000` (LQ) / +0x5c `.word 0x7C620000` (SQ)   a1 -> v1 = s0+0x60
     +0x64 `.word 0x7E000000` (SQ)                                   s0+0x50 <- zero
   MEASURED this session on func_L03_0041F048: this cc1 expands a 16-byte aggregate
   copy into ld/sd word moves and never emits LQ/SQ, so no C can produce them.
   ASM_FUNC candidate. (The `$gp` load of `lw $a0,0x1148($gp)` is a normal member;
   only the three block moves are out of reach.) */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_003D0238);
