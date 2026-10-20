/* 0 of 2 places match - 128-bit LQ/SQ pair (out of C reach).
   Level 2 (02_maktar_nebula), 0x003A8770, 148 bytes: ordinary field/flag stores and
   two frame calls (func_00314170, func_002F1F38, func_002F27F0) plus one
   `.word 0x7A420000` (opcode 0x1E = LQ) and `.word 0x7C820000` (opcode 0x1F = SQ)
   in the middle of the body. rabbitizer prints both as `.word # INVALID`.
   MEASURED this session on func_L03_0041F048: this cc1 expands a 16-byte aggregate
   copy into ld/sd word moves (92 bytes), it never emits LQ/SQ, so no C input can
   produce these two instructions. Needs ASM_FUNC. */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_003A8770);
