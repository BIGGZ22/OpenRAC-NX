/* 0 of 3 places match - 128-bit LQ/SQ (out of C reach).
   Level 2 (02_maktar_nebula), 0x00348420, 180 bytes: a nearest-index loop over
   0x10-byte records (func_2F2130, c.olt.s against 0x4B18967F = 9999999.0f,
   bc1f/mov.s to keep the running minimum). The block is blocked only by
   `.word 0x78820000` (opcode 0x1E = LQ) and `.word 0x7FA20000` (opcode 0x1F = SQ)
   at +0x38/+0x3c, which move the loop's scratch record on and off the stack.
   MEASURED this session on func_L03_0041F048: this cc1 expands a 16-byte aggregate
   copy into ld/sd word moves and never emits LQ/SQ, so no C can produce them.
   ASM_FUNC candidate. */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_00348420);
