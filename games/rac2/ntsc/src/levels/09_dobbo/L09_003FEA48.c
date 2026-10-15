#include "common.h"

extern void func_L08_00328BB0();
extern void func_L01_00321178();
extern void func_002B4FF0(f32 a);
extern char *D_L09_001B285C __attribute__((sda));
extern char *D_L09_001FFFB560[];
extern void func_2EA110(char *p);
extern void func_31F500(char *a0, char *a1, s32 a2, f32 f12, f32 f13);
extern void func_322578(char *a0, char *a1, s32 a2, s32 a3, f32 f12);
extern void func_L09_004081F0(char *a0, s32 a1, char *a2, s32 a3);
extern void func_L09_00408138(char *a0, char *a1, s32 a2, s32 a3, s32 a4);
extern s32 func_325E78(char *a0, s32 a1);
extern void func_318450(char *a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                        f32 f12, f32 f13, f32 f14);
extern char D_L09_00189EA0[];
extern void func_L09_00406BA0(s32 a0, u8 a1);

/* Level 09 helpers. */
void func_L09_00403080(char *a0) {
    s16 *p;
    s32 v;
    char *a1;
    char *a2;
    if (*(u8 *)(a0 + 0x21) == 0xFF) {
        return;
    }
    v = (s32)D_L09_001FFFB560[*(u8 *)(a0 + 0x21)];
    if (v == 0) {
        return;
    }
    p = (s16 *)v;
    a2 = D_L09_001B285C;
    do {
        a1 = a2 + ((*(u16 *)p & 0x7FFF) << 8);
        if (*(s8 *)(a1 + 0x20) >= 0) {
            if (*(s16 *)(a1 + 0xAA) == 0xD57) {
                *(u16 *)(*(char **)(a1 + 0x68) + 0x270) = 1;
            }
        }
    } while (*(s16 *)p++ >= 0);
}

void func_L09_00405060(char *a0) {
    s16 *p;
    s32 v;
    char *a1;
    char *a2;
    if (*(u8 *)(a0 + 0x21) == 0xFF) {
        return;
    }
    v = (s32)D_L09_001FFFB560[*(u8 *)(a0 + 0x21)];
    if (v == 0) {
        return;
    }
    p = (s16 *)v;
    a2 = D_L09_001B285C;
    do {
        a1 = a2 + ((*(u16 *)p & 0x7FFF) << 8);
        if (*(s8 *)(a1 + 0x20) >= 0) {
            if (*(s16 *)(a1 + 0xAA) == 0xD90) {
                *(u16 *)(*(char **)(a1 + 0x68) + 0x336) = 1;
            }
        }
    } while (*(s16 *)p++ >= 0);
}

void func_L09_00408250(char *a0, char *a1, s32 a2, s32 a3) {
    s16 *p;
    p = (s16 *)D_L09_001FFFB560[a2];
    if (p == 0) {
        return;
    }
    do {
        func_L09_004081F0(a0, *(u16 *)p & 0x7FFF, a1, a3);
    } while (*(s16 *)p++ >= 0);
}

void func_L09_00404FC8(char *a0) {
    char *s1 = *(char **)(a0 + 0x68);
    u16 n;
    func_2EA110(s1 + 0x330);
    func_2EA110(s1 + 0x332);
    n = *(u16 *)(a0 + 0x96);
    if (n < 0xFFFF) {
        *(u16 *)(a0 + 0x96) = n + 1;
    }
    func_31F500(a0, s1 + 0x280, 0, 0.03f, 0.3f);
    func_322578(a0, s1 + 0x360, 0, 1, 1.5f);
}

void func_L09_00403378(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    if (*(u8 *)(a0 + 0x20) == 9) {
        return;
    }
    *(f32 *)(a0 + 0x18) = *(f32 *)(p + 0x288);
}

void func_L09_00406BA0(s32 a0, u8 a1) {
    char *v1;
    if (a0 == -1) {
        return;
    }
    v1 = D_L09_001B285C;
    v1 += a0 << 8;
    if (v1 == 0) {
        return;
    }
    if (*(u8 *)(v1 + 0x20) == 0xFE) {
        return;
    }
    if (*(u8 *)(v1 + 0x20) == 0xFD) {
        return;
    }
    if (*(s16 *)(v1 + 0xAA) == 0x102 || *(s16 *)(v1 + 0xAA) == 0xDE0) {
        *(u8 *)(v1 + 0x95) = a1;
    }
}

void func_L09_00406C68(s32 a0) {
    if (a0 == -1) {
        return;
    }
    func_L08_00328BB0(a0);
}

void func_L09_00404F90(char *a0, s32 a1, s32 a2) {
    char *p = *(char **)(a0 + 0x68);
    if (a2 == 0) {
        func_L01_00321178();
    } else {
        func_002B4FF0(*(f32 *)(p + 0x33C));
    }
}

INCLUDE_ASM("asm/levels", func_L09_003FEA48);

INCLUDE_ASM("asm/levels", func_L09_003FEAE8);

INCLUDE_ASM("asm/levels", func_L09_003FECF0);

INCLUDE_ASM("asm/levels", func_L09_003FEF00);

INCLUDE_ASM("asm/levels", func_L09_00402D50);

INCLUDE_ASM("asm/levels", func_L09_00403100);

extern char D_L09_001B9C30[];
extern f32 func_2EA578(char *a0, char *a1);
extern f32 func_2EA3E0(f32 x);
extern void func_2EACD0(char *a0, char *a1);

void func_L09_0040B008(char *a0) {
    char *s1 = a0;
    char *s2 = s1 + 0xC0;
    char *s3 = *(char **)(s1 + 0x68);
    f32 a, b;
    *(s32 *)(s3 + 0x8) = 0;
    *(s32 *)(s3 + 0xC) = 0;
    *(f32 *)(s3 + 0x0) = 0.2f;
    *(f32 *)(s3 + 0x4) = 0.2f;
    func_2EACD0(s2, s1 + 0xF0);
    a = func_2EA3E0(func_2EA578(s2, D_L09_001B9C30));
    b = func_2EA3E0(func_2EA578(s1 + 0xD0, D_L09_001B9C30));
    if (b < a) {
        *(s32 *)(s3 + 0x14) = 1;
    } else {
        *(s32 *)(s3 + 0x14) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L09_00403270);

INCLUDE_ASM("asm/levels", func_L09_00403398);
INCLUDE_ASM("asm/levels", func_L09_00403700);

INCLUDE_ASM("asm/levels", func_L09_00403798);

INCLUDE_ASM("asm/levels", func_L09_00404C50);

INCLUDE_ASM("asm/levels", func_L09_00404E58);

INCLUDE_ASM("asm/levels", func_L09_004050E0);

INCLUDE_ASM("asm/levels", func_L09_004052F8);

INCLUDE_ASM("asm/levels", func_L09_00405490);

INCLUDE_ASM("asm/levels", func_L09_00405610);
INCLUDE_ASM("asm/levels", func_L09_00406BF8);

void func_L09_00406C90(char *a0) {
    char *a1 = *(char **)(a0 + 0x68);
    s32 v1 = *(s32 *)(a1 + 0x5C);
    if (v1 == -1) {
        return;
    }
    *(u8 *)((char *)((v1 << 8) + (s32)D_L09_001B285C) + 0xBC) = 1;
    *(s32 *)(a1 + 0x5C) = -1;
}


INCLUDE_ASM("asm/levels", func_L09_00406CC0);

INCLUDE_ASM("asm/levels", func_L09_00406E78);

INCLUDE_ASM("asm/levels", func_L09_00406FC8);
INCLUDE_ASM("asm/levels", func_L09_00408058);

extern void func_L09_00408058(char *a0, s32 a1, s32 a2, s32 a3);

/* Sets an entity's colour key, or hands an off-kind entity to the shared body. */
void func_L09_00408138(char *a0, char *a1, s32 a2, s32 a3, s32 a4)
{
    u8 c;
    s16 kind;
    char *p;

    if (a1 == 0) {
        return;
    }
    c = *(u8 *)(a1 + 0x20);
    if (c == 0xFE) {
        return;
    }
    if (c == 0xFD) {
        return;
    }
    kind = *(s16 *)(a1 + 0xAA);
    if (kind == 0xD0F) {
        p = *(char **)(a1 + 0x68);
        *(u8 *)(p + 0x31C) = a2;
        c = *(u8 *)(a1 + 0x30);
        if (c != 0xFF) {
            *(u8 *)(p + 0x31E) = c;
        }
        if (a2 != 0) {
            *(u8 *)(a1 + 0x30) = 0xFF;
        }
        return;
    }
    if (kind == 0xD09 || kind == 0xCEA) {
        func_L09_00408058(a1, 0, a3, a4);
        return;
    }
    if (kind == 0xDE0) {
        *(u8 *)(a1 + 0x95) = *(u8 *)(a0 + 0x95);
        return;
    }
    func_L09_00408058(a1, a2, a3, a4);
}
INCLUDE_ASM("asm/levels", func_L09_004081F0);

INCLUDE_ASM("asm/levels", func_L09_004082D0);

INCLUDE_ASM("asm/levels", func_L09_00408478);

INCLUDE_ASM("asm/levels", func_L09_004091C8);

INCLUDE_ASM("asm/levels", func_L09_00409308);

INCLUDE_ASM("asm/levels", func_L09_0040A2E8);

INCLUDE_ASM("asm/levels", func_L09_0040AC48);

INCLUDE_ASM("asm/levels", func_L09_0040B0C8);

INCLUDE_ASM("asm/levels", func_L09_0041D650);

INCLUDE_ASM("asm/levels", func_L09_00421B80);

INCLUDE_ASM("asm/levels", func_L09_00421D60);
