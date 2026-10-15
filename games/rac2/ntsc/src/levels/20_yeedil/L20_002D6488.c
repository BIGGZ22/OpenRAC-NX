#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L20_002D6488);

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
