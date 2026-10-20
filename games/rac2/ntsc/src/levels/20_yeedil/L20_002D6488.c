#include "include_asm.h"

/* Near miss: nonmatching/levels/func_L20_002D6488.c - the body needs -G8 (it
   reaches three globals through $gp), and the note there now explains the
   retail's split 64-bit "la" (the %hi half in a callee-saved register, the
   address re-made at every use) and the C spelling that produces it: a
   second C name for the same global. */
INCLUDE_ASM("asm/levels", func_L20_002D6488);

/* Not attempted: nonmatching/levels/func_L20_002D7730.c holds a map of this
   body (19548 bytes, 277 blocks, 238 calls, no loop; the dispatch, the join
   points, the exits, the $gp accesses, and the -G8 unit it needs). */
INCLUDE_ASM("asm/levels", func_L20_002D7730);

extern unsigned char D_L20_0018C0D4;

extern void func_L20_002D7730(int a0, int a1);

/* Picks the variant of the shared loader the current game state selects. */
void func_L20_002DC390(int a0)
{
    int a1 = (a0 != 0) ? 2 : 1;

    switch (D_L20_0018C0D4) {
    case 0:
        func_L20_002D7730(0, a1);
        break;
    case 1:
        func_L20_002D7730(0x3F, a1);
        break;
    case 4:
        func_L20_002D7730(0x8D, a1);
        break;
    }
}
