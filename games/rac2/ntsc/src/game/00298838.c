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

extern int D_001393E0[];
extern int D_001A8BB0[] __attribute__((sda));
extern int ref_L00_00309C18_00 __attribute__((sda));
extern int func_002B49E0(int, int, unsigned int, unsigned int, unsigned char *);
extern void func_00288D70(void);
void func_00298A20(void)
{
    int nextmode;
    int *state = D_001393E0;
    if (state[0x15C / 4] == 2 && state[0x164 / 4] < 0) {
        int mode = state[0x16C / 4];
        if (mode != 0 || state[0x10 / 4] != 0) {
            state[0x17C / 4] = 0;
            if (mode != 0) {
                D_001A7420[0] = 21;
                D_001A7424[0] = 0x40;
                if (D_001A8BB0[0] == 0) {
                    func_002B49E0(4, 1, 1, 0, 0);
                    mode = ref_L00_00309C18_00;
                } else {
                    func_002B49E0(4, 2, 1, 0, 0);
                    mode = D_001A8BB0[0];
                }
                if (mode == 6) func_00288D70();
            } else {
                *(volatile int *)D_001A7424 = (*(int *)D_001A7424 & ~0x40) | 1;
            }
            nextmode = 2;
        } else {
            nextmode = 1;
            state[0x17C / 4] = nextmode;
        }
        D_001A7420[0] = nextmode;
    }
}


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

extern unsigned char D_001A7930[];
extern unsigned char D_001A7948[];
extern unsigned char D_001A7960[];
extern unsigned char D_001A7980[];
extern unsigned char D_001A79A0[];
extern unsigned char D_001A79D0[];
extern void func_00115AC0(void *, int, int);
void func_00298BB8(unsigned char *source)
{
    int i;
    int value = source[0x12];
    if (value == 0x45)
        D_001A7930[2] = value;
    else if (value == 0x50)
        D_001A7930[2] = 0x49;
    for (i = 3; i < 7; i++)
        D_001A7930[i] = source[i + 0xd];
    for (i = 8; i < 11; i++)
        D_001A7930[i] = source[i + 0xd];
    for (i = 11; i < 13; i++)
        D_001A7930[i] = source[i + 0xe];
    func_00115AC0(D_001A7948, (int)D_001A7930, 13);
    func_00115AC0(D_001A7960, (int)D_001A7930, 13);
    func_00115AC0(D_001A7980, (int)D_001A7930, 13);
    func_00115AC0(D_001A79A0, (int)D_001A7930, 13);
    func_00115AC0(D_001A79A0 + 0x14, (int)D_001A7930, 13);
    func_00115AC0(D_001A79D0, (int)D_001A7930, 13);
}

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
