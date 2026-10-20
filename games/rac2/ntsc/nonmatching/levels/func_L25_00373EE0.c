/* 0 of 1 places match - the retail bytes at the entry are not a function body.
   Level 25 (25_wupash_nebula), catalogue entry 0x00373EE0, size 0x14:
     00373ED8  jr    $ra                 <- previous entry (func_L00_004272A0, 8 bytes)
     00373EDC  lw    $v0,-0x2B18($gp)    <- its delay slot
     00373EE0  daddu $v0,$zero,$zero     <- this entry
     00373EE4  nop
     00373EE8  sw    $a0,0x34($v0)
     00373EEC  nop
     00373EF0  sw    $a0,0x3C($v0)
     00373EF4  nop
     00373EF8  lui   $v1,0x22            <- next entry (func_L05_00379F38, 12 bytes)
   No jr $ra anywhere in the 20 bytes, no prologue, and both stores go through
   $v0 = 0 (absolute 0x34 / 0x3C). Words are interleaved with nops, and the entry
   clobbers the value the *previous* function's delay slot just loaded -- the
   window does not compose into a program, so no C input can produce it. The
   neighbouring entries of this level-25 window are equally tiny (4 / 8 / 12
   bytes); this looks like a level-25 stub/data table, not compiler output.
   Data point for the maintainers: the catalogue entry, not the C, is what has to
   change. */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L25_00373EE0);
