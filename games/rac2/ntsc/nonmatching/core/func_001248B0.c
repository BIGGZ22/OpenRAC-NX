/* from src/core/00124568.c: the catalogue row is 68 bytes but the function is 60.
   Its first two words are the linker remnant rac1-decomp names func_00120C58 and keeps
   with LINKER_REMNANT (`addiu $sp,$sp,0x40` - the surviving delay slot of the `jr $ra`
   of a function the linker dead-stripped, whose 0x40 frame it pops - and the alignment
   nop), and the real body runs from 0x001248B8. symbol_addrs/usa-v2 says the same of
   the row: "libcdvd SIF command handler in cod/022FA8, whose address func_001248F8
   (compiled C since task 1900) installs as func_001248B0 + 8".

   The body is that handler: func_001248F8 hands its address to r1_func_0011AA38 as
   `(char *)r1_func_00120C58 + 8`, i.e. 0x001248B8 (retail builds it as one %hi/%lo pair:
   `lui $a1,0x12; addiu $a1,$a1,0x48B8`). It calls the pointer at 0x00141844 with the
   argument at 0x00141848, but only while the flag at 0x001363A4 is clear - the flag
   func_001248F8 raises around that call.

   The C below reproduces those 60 bytes at 0x001248B8 exactly. At the row's own address
   tools/check.py prints "boot: 60 bytes, expected 68" and --diff lines the candidate up
   instruction for instruction two words behind retail; placed at 0x001248B8 the words
   match one for one and the three globals resolve to 0x00141844, 0x00141848 and
   0x001363A4. The C cannot reach the row's address: it would have to start with the
   remnant's `addiu $sp,$sp,0x40; nop`, and no C emits a frame teardown as its first
   instruction.

   Unblocked by splitting the row in tools/levels.py (start it at 0x001248B8, 60 bytes,
   and give 0x001248B0..0x001248B7 to the stripped sibling or to a filler row), not by
   more C. The globals keep their rac1 names here, as in src/core/00124568.c; their rac2
   addresses are 0x00141844, 0x00141848 and 0x001363A4. */
#include "common.h"

extern void *r1_D_00159844;

extern int r1_D_00159848;

extern int r1_D_001313E4;

void func_001248B0(void) {
    if (r1_D_00159844 != 0 && r1_D_001313E4 == 0) {
        ((void (*)(int))r1_D_00159844)(r1_D_00159848);
    }
}
