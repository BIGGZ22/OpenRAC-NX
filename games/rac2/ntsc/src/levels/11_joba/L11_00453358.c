#include "common.h"

extern void func_00282D30_2(char *dst, char *src, float scale);
extern void func_002A7750(void *a0, int a1, int a2);
extern void func_002AB8C0(void *a0, void *a1);
extern void func_002A0488(void *a0, int a1, int a2, int a3);
extern void func_002A8C28(void *a0, void *a1, int a2, float f12, float f13, float f14,
                          int a3, int t0, int t1);
extern void func_L11_00459F30();
extern void func_L11_0045A3C8();
extern void func_L11_0045A578();
extern float func_00283248(float a, float b);
extern float func_002A7878(float a, float b);
extern float func_00283180(float x);
extern float func_00283198(float x);

INCLUDE_ASM("asm/levels", func_L11_00453358);

INCLUDE_ASM("asm/levels", func_L11_00454508);

void func_L11_00454AE0(char *p)
{
    char *q = *(char **)(p + 0x68);
    unsigned short c;

    func_002AB8C0(p, q + 0x50);
    if (*(short *)(q + 0x50) == 0) {
        if (*(short *)(q + 0x1FE) == 0) {
            func_002A0488(p, *(int *)(q + 0x1F0), *(int *)(q + 0x1F4), *(int *)(q + 0x1F8));
            *(short *)(q + 0x1FE) = 1;
        }
    } else {
        *(short *)(q + 0x1FE) = 0;
    }
    c = *(unsigned short *)(p + 0x96);
    if (c <= 0xFFFE)
        *(unsigned short *)(p + 0x96) = c + 1;
}

INCLUDE_ASM("asm/levels", func_L11_00454B60);

INCLUDE_ASM("asm/levels", func_L11_00454C78);

INCLUDE_ASM("asm/levels", func_L11_00454DD8);

INCLUDE_ASM("asm/levels", func_L11_00454F18);

INCLUDE_ASM("asm/levels", func_L11_00455DF8);

void func_L11_00456010(char *p)
{
    unsigned short c = *(unsigned short *)(p + 0x96);

    if (c < 0xFFFF)
        *(unsigned short *)(p + 0x96) = c + 1;
    func_002A8C28(p, p + 0x10, 0x4000000, 1.6f, 1.0f, 1.0f, 0, 1, 0);
}

INCLUDE_ASM("asm/levels", func_L11_00456070);

INCLUDE_ASM("asm/levels", func_L11_00456378);

INCLUDE_ASM("asm/levels", func_L11_00456438);

INCLUDE_ASM("asm/levels", func_L11_00456540);

void func_L11_004565D8(char *p, char *a1)
{
    float dx = *(float *)(p + 0x10) - *(float *)(a1 + 0x0);
    float dz = *(float *)(p + 0x14) - *(float *)(a1 + 0x4);
    float ang = func_00283248(dx, dz);
    char *q;
    float amp;

    ang = ang + func_002A7878(-0.78539825f, 0.78539825f);
    amp = func_002A7878(-2.0f, 2.0f) + 8.0f;
    q = *(char **)(p + 0x68);
    qcopy(q + 0x230, a1);
    *(float *)(q + 0x230) = *(float *)(q + 0x230) + amp * func_00283180(ang);
    *(float *)(q + 0x234) = *(float *)(q + 0x234) + amp * func_00283198(ang);
}

INCLUDE_ASM("asm/levels", func_L11_004566C0);

INCLUDE_ASM("asm/levels", func_L11_00456938);

INCLUDE_ASM("asm/levels", func_L11_00456A48);

INCLUDE_ASM("asm/levels", func_L11_00456C20);

INCLUDE_ASM("asm/levels", func_L11_00456EA0);

INCLUDE_ASM("asm/levels", func_L11_00457600);

INCLUDE_ASM("asm/levels", func_L11_004577F8);

INCLUDE_ASM("asm/levels", func_L11_004580E8);

INCLUDE_ASM("asm/levels", func_L11_00458A98);

INCLUDE_ASM("asm/levels", func_L11_00458C38);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00458D88(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

void func_L11_00458F58(char *p)
{
    *(int *)(p + 0x98) = 0;
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) | 0x41) & 0xEFFF;
    func_002A7750(p, 1, -1);
}

INCLUDE_ASM("asm/levels", func_L11_00458F90);

INCLUDE_ASM("asm/levels", func_L11_004590F0);

INCLUDE_ASM("asm/levels", func_L11_00459640);

INCLUDE_ASM("asm/levels", func_L11_00459D48);

void func_L11_00459E48(char *p)
{
    int i;

    func_L11_00459F30(p);
    func_L11_0045A3C8(p);
    for (i = 4; i >= 0; i--)
        func_L11_0045A578(p);
}

/* shared native body of family a04928c6 */
void func_L11_00459EA0(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L11_00459F30);

INCLUDE_ASM("asm/levels", func_L11_0045A3C8);

INCLUDE_ASM("asm/levels", func_L11_0045A578);

INCLUDE_ASM("asm/levels", func_L11_0045A8C0);

INCLUDE_ASM("asm/levels", func_L11_0045B5C8);

INCLUDE_ASM("asm/levels", func_L11_0045B6C8);

INCLUDE_ASM("asm/levels", func_L11_0045FA68);

INCLUDE_ASM("asm/levels", func_L11_0045FDB0);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00460038(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L11_004600A0);

INCLUDE_ASM("asm/levels", func_L11_00461308);
