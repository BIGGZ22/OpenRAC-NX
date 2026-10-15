#include "include_asm.h"

extern char D_L06_002AA500[];

extern void func_32D268(char *);
extern void func_366160(char *, char *, float, int, int);
extern void func_36C158();
extern void func_36C1B0(float);
extern void func_3BFBB8(char *, int);

extern char D_L06_00189EA0[];
extern char D_L06_001B13B0[];
extern char D_L06_001395CE;
extern void func_2E3EF0(char *);
extern int func_360F58(char *);
extern void func_35A550(char *, int, int);
extern void func_35B810(int, int);
extern int func_35CBA0(char *, int, int);
extern int func_3A0948(int, int, char *);
extern void func_3661E0(char *, int, float);
extern void func_350388(char *);
extern void func_32D650(char *, char *, float);
extern void func_L06_00457C00(char *, int, void *, int, float, float, float, float);
extern void func_32D5F8(char *, char *, char *);
extern void func_365030(char *, char *, char *, int);
extern float func_32DC80(float, float);
extern float func_32D740(char *);
extern void func_32D5C8(char *, char *, char *);
extern int func_32E728(float);
extern void func_352000(char *, int, int, int);
extern void func_32D8A8(char *, char *, float);
extern void func_00310C88();
extern int D_L06_001B375C __attribute__((sda));
extern int D_L06_001B3758 __attribute__((sda));
extern void func_L06_003DDCA8();
extern int func_32D238(char *);
extern int func_35A620(int);
extern void func_35A270(char *, int, int, int);
extern char *func_31B0D0(char *, int, int, int, float, float, char *, char *, char *,
                         int);
extern char D_L06_001A8F20[];
extern void func_L06_004579C8(char *);

INCLUDE_ASM("asm/levels", func_L06_00457920);

INCLUDE_ASM("asm/levels", func_L06_0041FAF8);

void func_L06_004579C8(char *a0)
{
    char *s2 = *(char **)(a0 + 0x68);
    int v;

    func_32D5C8(a0 + 0x10, a0 + 0x10, s2);
    v = func_32E728((float)D_L06_001B375C * 1.5f);
    func_352000(a0, v, func_32E728((float)D_L06_001B375C * 1.25f), D_L06_001B375C);
    if (D_L06_001B3758 != 0) {
        func_32D8A8(s2, s2, 0.11666667461395264f);
        *(float *)(s2 + 0x8) = *(float *)(s2 + 0x8) + 0.18333333730697632f;
        *(unsigned int *)(a0 + 0x64) = (unsigned int)func_L06_003DDCA8;
        *(float *)(a0 + 0x2C) = *(float *)(a0 + 0x2C) * 0.75f;
        func_352000(a0, 0x10, 0x10, 0x10);
    }
}

void func_L06_00444068(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);
    float v[3];

    func_32D5F8((char *)v, D_L06_00189EA0, a0 + 0x10);
    func_365030(a0, (char *)v, (char *)v, 0);
    *(float *)(s1 + 0x2C0) = func_32DC80(v[0], v[1]);
    if (*(float *)(s1 + 0x2C0) > 0.78539825f) {
        *(float *)(s1 + 0x2C0) = 0.78539825f;
    } else if (*(float *)(s1 + 0x2C0) < -0.78539825f) {
        *(float *)(s1 + 0x2C0) = -0.78539825f;
    }
    *(float *)(s1 + 0x2C8) = -func_32DC80(func_32D740((char *)v), v[2]);
    if (*(float *)(s1 + 0x2C8) > 0.52359885f) {
        *(float *)(s1 + 0x2C8) = 0.52359885f;
    } else if (*(float *)(s1 + 0x2C8) < -0.52359885f) {
        *(float *)(s1 + 0x2C8) = -0.52359885f;
    }
}

void func_L06_00457AD0(unsigned char *a0)
{
    char *o = *(char **)(a0 + 0x68);

    switch (a0[0x20]) {
    case 0:
        *(unsigned short *)(a0 + 0x34) |= 0x800;
        func_35A550((char *)a0, 1, -1);
        break;
    case 1:
        if (func_35CBA0((char *)a0, 0x4010000, 0) != 0) {
            if (*(int *)o == 0) {
                func_35A550((char *)a0, 2, -1);
            }
        }
        a0[0xA8] = 0xFF;
        break;
    case 2: {
        char tmp[16];

        func_32D650(tmp, (char *)(a0 + 0xD0), -0.033333335f);
        func_L06_00457C00((char *)a0, 7, tmp, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f);
        func_3A0948(0, 0, (char *)a0);
        func_3661E0((char *)a0, 1, 0.0f);
        func_350388((char *)a0);
        break;
    }
    }
}

void func_L06_00457DA0(unsigned char *a0)
{
    char *o = *(char **)(a0 + 0x68);

    switch (a0[0x20]) {
    case 0:
        *(unsigned short *)(a0 + 0x34) |= 0x800;
        func_35A550((char *)a0, 1, -1);
        break;
    case 1:
        if (func_35CBA0((char *)a0, 0x4010000, 0) != 0) {
            if (*(int *)o == 0) {
                func_35A550((char *)a0, 2, -1);
            }
        }
        a0[0xA8] = 0xFF;
        break;
    case 2: {
        char tmp[16];

        func_32D650(tmp, (char *)(a0 + 0xD0), -0.033333335f);
        func_L06_00457C00((char *)a0, 7, tmp, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f);
        func_3A0948(0, 0, (char *)a0);
        func_3661E0((char *)a0, 1, 0.0f);
        func_350388((char *)a0);
        break;
    }
    }
}

void func_L06_0041FD60(char *a0)
{
    if (*(unsigned char *)(a0 + 0x21) != 0xFF)
        func_35B810(*(unsigned char *)(a0 + 0x21), 1);
}

INCLUDE_ASM("asm/levels", func_L06_0041DED8);

INCLUDE_ASM("asm/levels", func_L06_0041EF08);

INCLUDE_ASM("asm/levels", func_L06_0041F2A0);

INCLUDE_ASM("asm/levels", func_L06_0041F558);

INCLUDE_ASM("asm/levels", func_L06_0041F818);


INCLUDE_ASM("asm/levels", func_L06_0041FC58);

INCLUDE_ASM("asm/levels", func_L06_0041FD90);

INCLUDE_ASM("asm/levels", func_L06_00420080);

INCLUDE_ASM("asm/levels", func_L06_00420230);

INCLUDE_ASM("asm/levels", func_L06_00420548);

INCLUDE_ASM("asm/levels", func_L06_00420740);

INCLUDE_ASM("asm/levels", func_L06_00420AE8);

INCLUDE_ASM("asm/levels", func_L06_0042B868);

INCLUDE_ASM("asm/levels", func_L06_00430318);

INCLUDE_ASM("asm/levels", func_L06_00430C60);

INCLUDE_ASM("asm/levels", func_L06_00443280);

INCLUDE_ASM("asm/levels", func_L06_00443A38);

void func_L06_00443DE0(char *a0, int a1, int a2)
{
    char *p = *(char **)(a0 + 0x68);

    if (a2 == 0) {
        func_36C158();
    } else {
        func_36C1B0(*(float *)(p + 0x31C));
    }
}

void func_L06_00443E18(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);
    unsigned int v;

    func_32D268(s1 + 0x310);
    func_32D268(s1 + 0x312);
    v = *(unsigned short *)(a0 + 0x96);
    if (v <= 0xFFFE)
        *(unsigned short *)(a0 + 0x96) = v + 1;
    func_366160(a0, s1 + 0x260, 2.5f, 0, 1);
}

INCLUDE_ASM("asm/levels", func_L06_00443F30);

INCLUDE_ASM("asm/levels", func_L06_00444168);

INCLUDE_ASM("asm/levels", func_L06_00444300);

INCLUDE_ASM("asm/levels", func_L06_0044B250);

INCLUDE_ASM("asm/levels", func_L06_0044B2D8);

INCLUDE_ASM("asm/levels", func_L06_0044B350);

INCLUDE_ASM("asm/levels", func_L06_004504C8);

INCLUDE_ASM("asm/levels", func_L06_00455BC0);

INCLUDE_ASM("asm/levels", func_L06_00456278);

INCLUDE_ASM("asm/levels", func_L06_00456BB0);

void func_L06_00456EE0(void)
{
    func_3BFBB8(D_L06_002AA500, 0x30);
}

INCLUDE_ASM("asm/levels", func_L06_00456F08);

INCLUDE_ASM("asm/levels", func_L06_00457100);

INCLUDE_ASM("asm/levels", func_L06_00457270);

INCLUDE_ASM("asm/levels", func_L06_004573E8);

INCLUDE_ASM("asm/levels", func_L06_004576C8);

INCLUDE_ASM("asm/levels", func_L06_004577B0);



INCLUDE_ASM("asm/levels", func_L06_00457C00);

INCLUDE_ASM("asm/levels", func_L06_00457ED0);
