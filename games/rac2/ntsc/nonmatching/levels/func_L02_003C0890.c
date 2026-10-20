/* 0 of 1 places match - 128-bit LQ/SQ pair (out of C reach).
   Level 2 (02_maktar_nebula), 0x003C0890, 180 bytes: an object initialiser (flag
   bytes, a float copied from self+0xF8 to s1+0x10, func_314170's handle stored at
   s1+0x1C and its fields filled) whose middle holds
   `.word 0x7A030010` (opcode 0x1E = LQ, `lq $v1,0x10($s0)`) and
   `.word 0x7C430010` (opcode 0x1F = SQ, `sq $v1,0x10($v0)`) -- a 16-byte copy into
   the new object. MEASURED this session on func_L03_0041F048: this cc1 expands a
   16-byte aggregate copy into ld/sd word moves and never emits LQ/SQ, so no C can
   produce these two instructions. ASM_FUNC candidate. */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_003C0890);
