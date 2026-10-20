/* func_L04_0034B9B0, 12 bytes, 2 placements.  ABANDON - catalogue defect, the real body is
   eight bytes further on.
   The recorded extent opens on a stray epilogue fragment of the function before it: the
   previous one ends `jr $ra; addiu $sp,$sp,0x100`, and the extent then reads
   `addiu $sp,$sp,0x10; nop` -- neither is a prologue. From +8 (0x0034B9B8) the words are
   `addiu $sp,$sp,0xB0; nop; jr $ra; sw $a1,0xB8($a0)`, which is not a body either, so the
   boundary is wrong at both ends and the entry should be re-derived from the neighbour rather
   than attempted. Data for the maintainers: this is the `addiu $sp,$sp,+N; nop` family the
   brief lists, one of four in this lot.
   No C is deposited. */

#include "include_asm.h"
