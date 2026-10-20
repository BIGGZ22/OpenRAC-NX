/* from src/core/00127040.c: the catalogue row is 112 bytes but the function is 104.
   Its first two words are the linker remnant rac1-decomp names func_001235C0 and keeps
   with LINKER_REMNANT (`sw $a0,0x0($v1)` through a $v1 no entry sets, then `nop`), and
   the USA v2.00 symbol table still lists func_00127218 as its own 8-byte entry.
   The C below is rac1-decomp's func_001235C8 and reproduces the 104 bytes at 0x00127220
   exactly: tools/check.py prints "104 bytes, expected 112" and --diff lines the candidate
   up instruction for instruction two words behind retail. Unblocked by splitting the row
   in tools/levels.py, not by more C. */
#include "common.h"

extern int func_00126ED8(int);

extern void func_00126E60(void *);

void func_00127218(int *p, int arg1) {
    int r = func_00126ED8(arg1);

    func_00126E60(p);
    if ((unsigned int)p[12] != 0xFFFFFFFFU) {
        p[12] = r;
    }
    p[8] = 0;
    p[0] = (p[0] & ~0xC) | 0x105;
}
