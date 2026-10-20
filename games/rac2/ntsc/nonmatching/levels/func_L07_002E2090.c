/* 0 of 1 places match - 128-bit LQ/SQ stores (out of C reach).
   Level 7 (07_siberius), 0x002E2090, 112 bytes. Two `.word 0x7C400000` /
   `.word 0x7C600000` (opcode 0x1F = SQ, EE 128-bit store) hold the two 16-byte
   writes to the stack temporaries; the rest of the body is ordinary COP1 float
   work (lwc1/sub.s/mul.s with 0x3ECCCCCD = 0.4f and neg.s), so only the SQ pair
   blocks the match.
   MEASURED (this session, on func_L03_0041F048, same family): a 16-byte aggregate
   copy `*dst = *src;` compiles to 92 bytes of ld/sd word moves here, NOT to lq/sq -
   this cc1 does not emit the 128-bit forms from C. Both functions therefore need
   ASM_FUNC (lead decision) unless a builtin/attribute for the EE vector moves is
   found. */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L07_002E2090);
