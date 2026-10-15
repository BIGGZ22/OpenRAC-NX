#include "include_asm.h"

typedef unsigned short u16;

extern void func_00282978(void *);
extern void func_002A0488(void *, int, int, int);
extern void func_002A7470(void *, int, int, int);
extern void func_002AB8C0(void *, void *);
extern void func_L00_002E6CF0(void *);
extern void func_L04_003A21F8(void *, int);
extern char D_L04_002AF6C0[];

extern void func_0029FCC0(void *, int, void *);
extern float func_00283180(float);
extern float func_00283198(float);
extern void func_00282CC0(void *, void *, void *);
extern void func_L04_003E53D0(void *, void *);
extern void func_L04_003E55E0(void *, void *);

typedef struct L04_003E5318_BUF {
    float f[6];
    int z;
} L04_003E5318_BUF;

INCLUDE_ASM("asm/levels", func_L04_003E5298);

void func_L04_003E5318(char *a0)
{
    char *s0 = a0;
    char *p = *(char **)(s0 + 0x68);
    char *q = *(char **)(p + 0x2AC);
    L04_003E5318_BUF buf;

    if (q != 0) {
        char *r = *(char **)(q + 0x68);

        func_0029FCC0(*(char **)(r + 0x288), 0, &buf);
        buf.f[4] = func_00283180(*(float *)(s0 + 0xF8)) * 0.1f;
        buf.f[5] = func_00283198(*(float *)(s0 + 0xF8)) * 0.1f;
        buf.z = 0;
        func_00282CC0(&buf, &buf, buf.f + 4);
        buf.f[2] = buf.f[2] - 0.05f;
        func_L04_003E53D0(s0, &buf);
        func_L04_003E55E0(s0, &buf);
    }
}

void func_L04_003E51F0(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);
    u16 v;

    func_002AB8C0(a0, s0 + 0x50);
    func_00282978(s0 + 0x2A0);
    func_00282978(s0 + 0x2A2);
    v = *(u16 *)(a0 + 0x96);
    if (v <= 0xFFFE) {
        *(u16 *)(a0 + 0x96) = v + 1;
    }
}

void func_L04_003E5250(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);

    if (*(char **)(s0 + 0x2AC) != 0) {
        func_002A7470(*(char **)(s0 + 0x2AC), 9, 0, 3);
        *(int *)(*(char **)(*(char **)(s0 + 0x2AC) + 0x68) + 0x27C) = 0;
        *(char **)(s0 + 0x2AC) = 0;
    }
}

void func_L04_003E7FE8(void)
{
    func_L04_003A21F8(D_L04_002AF6C0, 10);
}

INCLUDE_ASM("asm/levels", func_L04_003AB458);

INCLUDE_ASM("asm/levels", func_L04_003B8B60);

void func_L04_003E22B0(char *a0, int a1, int a2, int a3, int a4)
{
    char *s2 = *(char **)(a0 + 0x68);

    if (*(char **)(s2 + 0x2AC) != 0) {
        func_002A7470(a0, a1, a3, a4);
        func_002A7470(*(char **)(s2 + 0x2AC), a2, a3, a4);
    } else if (*(unsigned char *)(a0 + 0x43) != a1) {
        func_002A7470(a0, a1, a3, a4);
    }
}

INCLUDE_ASM("asm/levels", func_L04_003E2338);

INCLUDE_ASM("asm/levels", func_L04_003E2438);

INCLUDE_ASM("asm/levels", func_L04_003E4C00);

INCLUDE_ASM("asm/levels", func_L04_003E53D0);

INCLUDE_ASM("asm/levels", func_L04_003E55E0);

INCLUDE_ASM("asm/levels", func_L04_003E5B10);

INCLUDE_ASM("asm/levels", func_L04_003E7E68);

INCLUDE_ASM("asm/levels", func_L04_003E8010);

INCLUDE_ASM("asm/levels", func_L04_003EB8D8);

INCLUDE_ASM("asm/levels", func_L04_003EC140);

INCLUDE_ASM("asm/levels", func_L04_003ED3D8);

INCLUDE_ASM("asm/levels", func_L04_003ED740);

INCLUDE_ASM("asm/levels", func_L04_00406C00);

INCLUDE_ASM("asm/levels", func_L04_00407B80);

INCLUDE_ASM("asm/levels", func_L04_00407F80);

INCLUDE_ASM("asm/levels", func_L04_00412AF8);

INCLUDE_ASM("asm/levels", func_L04_00422240);

INCLUDE_ASM("asm/levels", func_L04_00422318);
