#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L01_00397110);

INCLUDE_ASM("asm/levels", func_L01_003973A0);

INCLUDE_ASM("asm/levels", func_L01_00397870);

INCLUDE_ASM("asm/levels", func_L01_00397950);

INCLUDE_ASM("asm/levels", func_L01_00397A28);

INCLUDE_ASM("asm/levels", func_L01_00397C98);

INCLUDE_ASM("asm/levels", func_L01_00397EB0);

extern int D_001A79F0 __attribute__((sda));

extern char D_0019B4A8[];

extern void func_L01_003115E8(int, int);

extern void func_002A7750(char *, int, int);

void func_L01_00398008(char *a2)
{
    unsigned short f = *(unsigned short *)(a2 + 0xB2);

    if (f & 1) {
        char *p = (char *)(((short)f >> 1) + (D_001A79F0 << 10)) + (int)D_0019B4A8;
        unsigned char c = *(unsigned char *)p;

        if (((c >> 4) & 7) != 7)
            *(unsigned char *)p = c + 0x10;
    } else {
        char *p = (char *)(((short)f >> 1) + (D_001A79F0 << 10)) + (int)D_0019B4A8;
        unsigned char c = *(unsigned char *)p;

        if ((c & 7) != 7)
            *(unsigned char *)p = c + 1;
    }

    if (*(unsigned char *)(a2 + 0x21) != 0xFF)
        func_L01_003115E8(*(unsigned char *)(a2 + 0x21), 1);
    else
        func_002A7750(a2, 1, -1);
}

INCLUDE_ASM("asm/levels", func_L01_003980D0);

INCLUDE_ASM("asm/levels", func_L01_00398938);

INCLUDE_ASM("asm/levels", func_L01_00398A90);

INCLUDE_ASM("asm/levels", func_L01_00398B50);

void func_L01_00399098(char *a0)
{
    int i;
    char **p;

    *(unsigned short *)(a0 + 0x34) |= 1;
    *(unsigned char *)(a0 + 0x31) = 0;
    *(unsigned int *)(a0 + 0x98) = 0;
    p = (char **)(*(char **)(a0 + 0x68) + 0x14);
    for (i = 2; i >= 0; i--) {
        if (*p != 0) {
            *(unsigned short *)(*p + 0x34) |= 1;
            *(unsigned char *)(*p + 0x31) = 0;
        }
        p++;
    }
}

INCLUDE_ASM("asm/levels", func_L01_003990E8);

INCLUDE_ASM("asm/levels", func_L01_00399170);

INCLUDE_ASM("asm/levels", func_L01_003993C8);

INCLUDE_ASM("asm/levels", func_L01_00399510);

INCLUDE_ASM("asm/levels", func_L01_003B2598);

INCLUDE_ASM("asm/levels", func_L01_003B9798);

INCLUDE_ASM("asm/levels", func_L01_003BD178);

INCLUDE_ASM("asm/levels", func_L01_003C91C8);

INCLUDE_ASM("asm/levels", func_L01_003CEB00);

INCLUDE_ASM("asm/levels", func_L01_003CF690);

INCLUDE_ASM("asm/levels", func_L01_003D2C88);

INCLUDE_ASM("asm/levels", func_L01_003D4270);

INCLUDE_ASM("asm/levels", func_L01_003D6B00);

INCLUDE_ASM("asm/levels", func_L01_003E9CF8);

INCLUDE_ASM("asm/levels", func_L01_003E9E40);

INCLUDE_ASM("asm/levels", func_L01_003EE5F8);

INCLUDE_ASM("asm/levels", func_L01_003EE6F0);

INCLUDE_ASM("asm/levels", func_L01_003EE8E0);

INCLUDE_ASM("asm/levels", func_L01_003FFFE8);

extern void func_002A7750(char *, int, int);

extern void func_L00_00326048(char *, char *, float, int, int);

extern int D_L01_001A8F40 __attribute__((sda));

void func_L01_004110E0(char *a0)
{
    unsigned char v = *(unsigned char *)(a0 + 0x20);
    char *p = *(char **)(a0 + 0x68);

    switch (v) {
    case 0:
        func_002A7750(a0, 1, -1);
        break;
    case 1:
        if (p == 0)
            break;
        if (D_L01_001A8F40 == 2)
            func_L00_00326048(a0, p + 0xC, 0.055268768f,
                              (int)0x80AABED2u, (int)0x80466464u);
        else
            func_L00_00326048(a0, p + 0xC, 0.037815473f,
                              (int)0x8096AAAAu, (int)0x80466464u);
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L01_004132D8);

INCLUDE_ASM("asm/levels", func_L01_00417090);

INCLUDE_ASM("asm/levels", func_L01_0041C890);

INCLUDE_ASM("asm/levels", func_L01_00423990);

INCLUDE_ASM("asm/levels", func_L01_0042A568);

INCLUDE_ASM("asm/levels", func_L01_0042A6F0);

INCLUDE_ASM("asm/levels", func_L01_0042DE20);

INCLUDE_ASM("asm/levels", func_L01_0042E0A8);

INCLUDE_ASM("asm/levels", func_L01_00446920);

extern void func_002A6CF0(void *, void *);

extern int ref_L01_004480C8_00;

void func_L01_004480C8(void *a0)
{
    func_002A6CF0(&ref_L01_004480C8_00, a0);
}
