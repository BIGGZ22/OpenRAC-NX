/* func_L01_002A7248, 128 bytes, 4 places.  ABANDON - one word short, tail scheduling.
   Read in full: with a nonzero argument and D_L01_00189E20[0x22BA] set, when the halfword
   at +0x24EC is zero it reads the byte of D_00139A28 at the index func_2AE958(0) returns
   and answers `*(int *)(D_002637A0 + c * 0xE0 + 0x28)`; every other path answers
   0x51 when the word at +0x24CC is 1 and 0 otherwise.
   The candidate is 124 bytes against 128 (one word short): the `movn $v0,$zero,$a0` tail
   after an `xori ...,1` is present, but this cc1 arranges the two `addiu $v0,$zero,0x51`
   materialisations and the shared exit differently - retail duplicates the 0x51 load into
   both the bnel delay slot and the second entry, and the candidate folds one away. Three
   spellings measured (&& chain with a ternary tail, nested ifs, result variable).
   What would unblock it: a shape that keeps the two 0x51 constants on separate paths. */

#include "include_asm.h"
