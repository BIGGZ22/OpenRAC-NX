#include "include_asm.h"

extern char D_00189E20_9[];

extern char D_001A63A8_3[];

extern short D_00152C2A;

extern void func_002B72E8_1();

extern void func_002E4AC8();

extern void func_00288EA0();

extern void func_002E6D88();

typedef unsigned char u8;

typedef int s32;

typedef struct {
 u8 prefix[0x30];s32 state;u8 gap0[0x1c];u8 *current;u8 gap1[0x138];s32 bank[2];
} NativeContext;

extern NativeContext D_L24_001BC3C0;

extern u8 D_L24_0028AAC0[];

/* One-shot init: raises an int flag, forces a state word outside {6,7} to 5, runs three calls, then clears a 16-bit global. */
void func_002E7018(void)
{
    char *p = D_00189E20_9;
    char *q;
    unsigned short v;

    if (*(int *)(p + 9420) <= 0) {
        *(int *)(p + 9420) = 1;
    }
    func_002B72E8_1(0);

    q = D_001A63A8_3;
    v = *(unsigned short *)(q + 114);
    if ((unsigned short)(v - 6) >= 2) {
        *(unsigned short *)(q + 114) = 5;
    }
    func_002E4AC8();
    func_00288EA0();
    func_002E6D88(0);
    D_00152C2A = 0;
}

INCLUDE_ASM("asm/game", func_002E7090);

INCLUDE_ASM("asm/game", func_002E71B8);

INCLUDE_ASM("asm/game", func_002E7248);

INCLUDE_ASM("asm/game", func_002E7338);

INCLUDE_ASM("asm/game", func_002E7570);

INCLUDE_ASM("asm/game", func_002E76C0);

INCLUDE_ASM("asm/game", func_002E7B48);

INCLUDE_ASM("asm/game", func_002E7BC0);

INCLUDE_ASM("asm/game", func_002E7BC8);

INCLUDE_ASM("asm/game", func_002E80B0);

INCLUDE_ASM("asm/game", func_002E8190);

INCLUDE_ASM("asm/game", func_002E82C8);

INCLUDE_ASM("asm/game", func_002E8368);

INCLUDE_ASM("asm/game", func_002E8BA8);

INCLUDE_ASM("asm/game", func_002E8F80);

INCLUDE_ASM("asm/game", func_002E9110);

INCLUDE_ASM("asm/game", func_002E9268);

INCLUDE_ASM("asm/game", func_002EA8E8);

INCLUDE_ASM("asm/game", func_002EA968);

INCLUDE_ASM("asm/game", func_002EAAA8);

INCLUDE_ASM("asm/game", func_002EAB18);

INCLUDE_ASM("asm/game", func_002EB150);

INCLUDE_ASM("asm/game", func_002EB158);

INCLUDE_ASM("asm/game", func_002EB1E0);

INCLUDE_ASM("asm/game", func_002EB598);

INCLUDE_ASM("asm/game", func_002EB7E8);

INCLUDE_ASM("asm/game", func_002EBAA8);

INCLUDE_ASM("asm/game", func_002EBC70);

INCLUDE_ASM("asm/game", func_002EBDF0);

INCLUDE_ASM("asm/game", func_002EC1C0);

INCLUDE_ASM("asm/game", func_002EC408);

INCLUDE_ASM("asm/game", func_002ECB70);

/* selects the level data bank when the resident context is in state 9 */
void func_002ED058(void) {
 s32 index=-1;
 if(D_L24_001BC3C0.state==9) index=1;
 if(index!=-1) {
  D_L24_001BC3C0.current=D_L24_0028AAC0;
  *(s32 *)(D_L24_0028AAC0+0x68)=D_L24_001BC3C0.bank[index];
 }
}

INCLUDE_ASM("asm/game", func_002ED0A0);
