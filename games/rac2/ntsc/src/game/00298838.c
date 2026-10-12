#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_60904fd170c92806_SDataMoreCleanup184Layout {
    unsigned char unknown_00[0x10];
    int selector_10;
    unsigned char unknown_14[0x168];
    int value_17c;
} Rac2Native_60904fd170c92806_SDataMoreCleanup184Layout;

extern Rac2Native_60904fd170c92806_SDataMoreCleanup184Layout ref_L00_00309A30_00;

extern int ref_L00_00309A30_01;

extern int ref_L00_00309A30_02;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_8391b53329533f81_SDataGuardedFlags112Layout {
    unsigned char unknown_00[0x24];
    int word_24;
    unsigned char unknown_28[0x134];
    int mode_15c;
    unsigned char unknown_160[0x4];
    int selected_164;
    unsigned char unknown_168[0x4];
    int extra_16c;
    unsigned char unknown_170[0xc];
    int value_17c;
} Rac2Native_8391b53329533f81_SDataGuardedFlags112Layout;

extern Rac2Native_8391b53329533f81_SDataGuardedFlags112Layout ref_L00_00309BA8_00;

extern unsigned int ref_L00_00309BA8_01;

extern int ref_L00_00309BA8_02;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_83cf4a65b6159c6c_SDataRootClear72Layout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_83cf4a65b6159c6c_SDataRootClear72Layout;

extern Rac2Native_83cf4a65b6159c6c_SDataRootClear72Layout ref_L00_00309D20_01;

extern int ref_L00_00309D20_00;

extern int ref_L00_00309D20_02;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_25b7bae55e05d925_SDataRootClear68Layout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_25b7bae55e05d925_SDataRootClear68Layout;

extern Rac2Native_25b7bae55e05d925_SDataRootClear68Layout ref_L00_00309D68_01;

extern int ref_L00_00309D68_00;

extern int ref_L00_00309D68_02;

void func_00298838(void)
{
    if ((ref_L00_00309A30_01 & 4) != 0)
        ref_L00_00309A30_01 &= ~4;
    if ((ref_L00_00309A30_01 & 2) != 0)
        ref_L00_00309A30_01 &= ~2;
    if ((ref_L00_00309A30_01 & 0x80) != 0) {
        ref_L00_00309A30_02 = 21;
        ref_L00_00309A30_01 = (ref_L00_00309A30_01 ^ 0x80) | 0x40;
        return;
    }
    if ((ref_L00_00309A30_01 & 0x100) != 0) {
        ref_L00_00309A30_02 = 20;
        ref_L00_00309A30_01 = (ref_L00_00309A30_01 ^ 0x100) | 0x40;
        return;
    }
    if (ref_L00_00309A30_00.selector_10 != 0) {
        ref_L00_00309A30_02 = 3;
        return;
    }
    if (ref_L00_00309A30_00.value_17c != 0)
        ref_L00_00309A30_02 = 1;
}

INCLUDE_ASM("asm/game", func_002988F0);

INCLUDE_ASM("asm/game", func_00298918);

INCLUDE_ASM("asm/game", func_00298940);

INCLUDE_ASM("asm/game", func_00298960);

INCLUDE_ASM("asm/game", func_00298988);

void func_002989B0(void)
{
    if (ref_L00_00309BA8_00.mode_15c != 2 || ref_L00_00309BA8_00.selected_164 >= 0)
        return;
    if (ref_L00_00309BA8_00.extra_16c != 0 || ref_L00_00309BA8_00.word_24 != 0) {
        ref_L00_00309BA8_00.value_17c = 0;
        ref_L00_00309BA8_02 = 21;
        ref_L00_00309BA8_01 |= 0x440u;
        return;
    }
    ref_L00_00309BA8_00.value_17c = 1;
    ref_L00_00309BA8_02 = 1;
}

INCLUDE_ASM("asm/game", func_00298A20);

void func_00298B28(void)
{
    if (ref_L00_00309D20_01.selector_10 != -2) {
        ref_L00_00309D20_02 = 3;
        return;
    }
    if ((ref_L00_00309D20_00 & 0x20) != 0) {
        ref_L00_00309D20_00 ^= 0x20;
        ref_L00_00309D20_02 = 5;
    }
}

void func_00298B70(void)
{
    if (ref_L00_00309D68_01.selector_10 != 0) {
        ref_L00_00309D68_02 = 3;
        return;
    }
    if ((ref_L00_00309D68_00 & 0x20) != 0) {
        ref_L00_00309D68_00 ^= 0x20;
        ref_L00_00309D68_02 = 12;
    }
}

INCLUDE_ASM("asm/game", func_00298BB8);

INCLUDE_ASM("asm/game", func_00298D10);

INCLUDE_ASM("asm/game", func_00298D28);

INCLUDE_ASM("asm/game", func_00298E08);

INCLUDE_ASM("asm/game", func_00298F00);

INCLUDE_ASM("asm/game", func_0029ADA0);

INCLUDE_ASM("asm/game", func_0029AE48);
