#include "include_asm.h"

extern int ref_L00_00312060_01;

extern unsigned char ref_L00_00312060_00[];

void func_002A0430(unsigned int index)
{
    if (index != 255u)
        ref_L00_00312060_00[index + (unsigned int)ref_L00_00312060_01 * 16u] |= 0x80u;
}

INCLUDE_ASM("asm/game", func_002A0468);

INCLUDE_ASM("asm/game", func_002A0488);

INCLUDE_ASM("asm/game", func_002A04B8);

INCLUDE_ASM("asm/game", func_002A04E8);

INCLUDE_ASM("asm/game", func_002A0558);

INCLUDE_ASM("asm/game", func_002A05A8);

INCLUDE_ASM("asm/game", func_002A08A0);

INCLUDE_ASM("asm/game", func_002A0A28);

INCLUDE_ASM("asm/game", func_002A0B88);

INCLUDE_ASM("asm/game", func_002A0BE8);

INCLUDE_ASM("asm/game", func_002A0C68);

INCLUDE_ASM("asm/game", func_002A0F48);

INCLUDE_ASM("asm/game", func_002A10E8);

INCLUDE_ASM("asm/game", func_002A1130);

INCLUDE_ASM("asm/game", func_002A1378);

INCLUDE_ASM("asm/game", func_002A1490);

INCLUDE_ASM("asm/game", func_002A1738);

INCLUDE_ASM("asm/game", func_002A2450);

INCLUDE_ASM("asm/game", func_002A3110);

INCLUDE_ASM("asm/game", func_002A3DB8);

INCLUDE_ASM("asm/game", func_002A3F10);

INCLUDE_ASM("asm/game", func_002A4078);

INCLUDE_ASM("asm/game", func_002A4AD8);

INCLUDE_ASM("asm/game", func_002A4D88);

INCLUDE_ASM("asm/game", func_002A4E68);

INCLUDE_ASM("asm/game", func_002A6640);

INCLUDE_ASM("asm/game", func_002A677C);

INCLUDE_ASM("asm/game", func_002A68A8);

INCLUDE_ASM("asm/game", func_002A6AE0);

INCLUDE_ASM("asm/game", func_002A6C58);

INCLUDE_ASM("asm/game", func_002A7398);

INCLUDE_ASM("asm/game", func_002A7470);

INCLUDE_ASM("asm/game", func_002A75E0);
