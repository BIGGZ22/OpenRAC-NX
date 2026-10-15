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

extern unsigned int D_001A7424[] __attribute__((sda));
extern int D_001A7420[] __attribute__((sda));

void func_002988F0(void) {
    if ((D_001A7424[0] & 0x40) == 0)
        D_001A7420[0] = 3;
}

void func_00298918(void) {
    if ((D_001A7424[0] & 0x40) == 0)
        D_001A7420[0] = 3;
}

extern int D_001393F0[];

void func_00298940(void) {
    if (D_001393F0[0] != 0)
        D_001A7420[0] = 3;
}

void func_00298960(void) {
    if ((D_001A7424[0] & 0x40) == 0)
        D_001A7420[0] = 3;
}

void func_00298988(void) {
    if ((D_001A7424[0] & 0x40) == 0)
        D_001A7420[0] = 3;
}

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

extern char D_001A9970[];
extern void func_0029B168(char *, int, int *);
extern void func_0029B628(void);
extern void func_0026FEB8(void *);
extern int D_00256138[];
extern char D_00250000[];
extern int func_0029AE78(int *);

void func_00298D28(int *a0)
{
    int len1 = func_0029AE78(D_00256138);
    int len2 = func_0029AE78((int *)(D_00250000 + 0x6368));
    int size = a0[0];
    int i;

    if (size != len1 || a0[1] != len2) {
        func_0026FEB8(D_001A9970);
        return;
    }
    a0 = (int *)((char *)a0 + 8);
    func_0029B168((char *)a0, 0, D_00256138);
    a0 = (int *)((char *)a0 + size);
    for (i = 0; i < 0x1C; i++) {
        func_0029B168((char *)a0, i, (int *)(D_00250000 + 0x6368));
        a0 = (int *)((char *)a0 + len2);
    }
    func_0029B628();
}

INCLUDE_ASM("asm/game", func_00298E08);

INCLUDE_ASM("asm/game", func_00298F00);

extern int D_00256138[];
extern char D_00250000[];
extern int func_0029AE78(int *);
extern int func_0029AF98(char *, int, int *);

void func_0029ADA0(char *a0)
{
    char *out = a0;
    int i = 0;

    *(int *)(out + 0) = func_0029AE78(D_00256138);
    *(int *)(out + 4) = func_0029AE78((int *)(D_00250000 + 0x6368));
    out += 8;
    out += func_0029AF98(out, 0, D_00256138);
    for (i = 0; i < 0x1C; i++) {
        out += func_0029AF98(out, i, (int *)(D_00250000 + 0x6368));
    }
}

extern int func_00127348(void);
extern char D_001A9A28[];
extern void func_0026FEB8(void *);

void func_0029AE48(void) {
    if (func_00127348() != 0)
        func_0026FEB8(D_001A9A28);
}
