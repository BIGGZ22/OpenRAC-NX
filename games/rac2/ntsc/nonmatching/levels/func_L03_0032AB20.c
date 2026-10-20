/* 0 of 3 places match - last line: 03_endako: 72 bytes, expected 80
   The best candidate is 72 bytes against 80: the first ten instructions match exactly,
   including the annulled `beql $v1,$zero,L; lw $v0,0x24CC($a0)` conditional-execute pair.
   Two shapes matter and are worth keeping:
     * the data base must be the file's own symbol `D_L03_00189E20` (-G0 file, unsigned
       char[]): with the literal 0x00189E20 the address is folded into every access and
       rematerialised, which also loses the annulled pair;
     * the boot word at 0x001A8FB4 is fine as a literal.
   The eight bytes that stay behind are block layout: retail sends all four failures to one
   return-0 block placed after the second test (the third test branches backward +0x20 to
   it), while this cc1 puts the third test's failure after the tests (forward branch) and
   folds the last comparison into a boolean (`xori`/`sltu`) instead of `beq` + `addiu
   $v0,$zero,0x1`.  A short-circuit `||` chain was tried: it hoists the pointer setup above
   the first test and loses the delay-slot fill, so it is further away, not closer. */

#include "include_asm.h"

extern unsigned char D_L03_00189E20[];

int func_L03_0032AB20(int arg)
{
    unsigned char *p;

    if (*(int *)0x001A8FB4 != arg)
        return 0;
    p = D_L03_00189E20;
    if (*(unsigned char *)(p + 0x123A) != 0)
        return 0;
    if (*(int *)(p + 0x24CC) == 0)
        return 0;
    if (*(int *)(p + 0x229C) == 0x14)
        return 0;
    return 1;
}
