#include "common.h"

extern char *D_L06_001B9890[];

extern char *func_00270280(int);
extern float func_002A8A20(char *);
extern int func_00273B70(char *);
extern void func_002A7750(char *, int, int);
extern void func_0029F338(char *);
extern int func_002A8600(char **, int, int, int);
extern int func_002A86E8(char **, char *, int, int);
extern void func_002A7470(char *, int, int, int);
extern void func_002AA548(char *, char *, float, float, float, float);
extern int func_L04_0033CB08(char *, char *, int, int, float, float);
extern void func_00282D30(char *, char *, float);
extern float func_00282E20(char *);
extern float func_00283C80(float, float);
extern float func_00283180(float);
extern float func_00283198(float);
extern int func_002A9640(char *, int, int);
extern void func_L06_00457C00(char *, int, void *, int, float, float, float, float);
extern void func_002E5CF8(int, int, char *);
extern void func_L00_00326110(char *, int, float);

INCLUDE_ASM("asm/levels", func_L06_00458138);

INCLUDE_ASM("asm/levels", func_L06_004583A8);

INCLUDE_ASM("asm/levels", func_L06_00458568);

INCLUDE_ASM("asm/levels", func_L06_004587E8);

INCLUDE_ASM("asm/levels", func_L06_00458E10);

INCLUDE_ASM("asm/levels", func_L06_00458F30);

INCLUDE_ASM("asm/levels", func_L06_00459020);

INCLUDE_ASM("asm/levels", func_L06_00459620);

INCLUDE_ASM("asm/levels", func_L06_00459828);

INCLUDE_ASM("asm/levels", func_L06_00459B18);

int func_L06_00459BA8(char *a0, char *a1)
{
    float f = func_002A8A20(a0 + 0x10);
    float t;

    *(float *)(a1 + 0x120) = f;
    t = *(float *)(*(char **)(a1 + 0x11C)) - f;
    *(float *)(a1 + 0x128) = f + t * 0.5f;
    if (t < 0.41f) {
        return 2;
    }
    if (0.68f < t) {
        return 0;
    }
    return 1;
}

void func_L06_00459C38(unsigned char *a0)
{
    switch (a0[0xBC]) {
    case 0:
        if (a0[0x43] != 0) {
            func_002A7470((char *)a0, 0, 0, 0xA);
        }
        a0[0x20] = 1;
        break;
    case 1:
        if (a0[0x43] != 3) {
            func_002A7470((char *)a0, 3, 0, 0xA);
        }
        a0[0x20] = 7;
        break;
    case 2:
        if (a0[0x43] != 9) {
            func_002A7470((char *)a0, 9, 0, 0xA);
        }
        a0[0x20] = 0xF;
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L06_00459D08);

INCLUDE_ASM("asm/levels", func_L06_00459EC0);

void func_L06_0045A0D0(char *a0, char *a1, float f, float g, float h)
{
    char buf[16];
    char *q = a1;

    if (*(float *)(*(char **)(a1 + 0x11C)) - 0.3f < *(float *)(a0 + 0x18)) {
        *(float *)(a1 + 0xC8) -= 0.163333335f;
    }
    qcopy(buf, a0 + 0x10);
    func_002AA548(buf + 8, q + 0xC8, *(float *)(q + 0x128), f, g, h);
    if (func_L04_0033CB08(a0, q + 0xC0, 0x400, 0x10, 1.0f, 0.0f) != 0) {
        func_00282D30(q + 0xC0, q + 0xC0, 0.5f);
    }
}

int func_L06_0045A1B0(char *a0, char *a1, float p1, float p2, float p3, float p4, float p5,
                      float p6, float p7)
{
    float x = func_00282E20(a1 + 0xC0);
    float y = func_00283C80(*(float *)(a0 + 0xF8), p1);
    int r = 0;

    if (y < p2) {
        x += p3;
        r = 1;
    } else if (p4 < y) {
        x -= p5;
    }
    if (p6 < x) {
        x = p6;
    } else if (x < p7) {
        x = p7;
    }
    *(float *)(a1 + 0xC0) = func_00283180(*(float *)(a0 + 0xF8)) * x;
    *(float *)(a1 + 0xC4) = func_00283198(*(float *)(a0 + 0xF8)) * x;
    return r;
}

INCLUDE_ASM("asm/levels", func_L06_0045A2D8);

INCLUDE_ASM("asm/levels", func_L06_0045A490);

int func_L06_0045A500(char *a0, int a1, int a2)
{
    char *found = 0;

    if (func_002A8600(&found, *(unsigned char *)(a0 + 0x21), 0, 0) != 0) {
        return 0;
    }
    while (found != 0) {
        if (found != a0) {
            int v = *(unsigned char *)(found + 0x20);

            if (v >= a1) {
                if (v <= a2) {
                    return 1;
                }
            }
        }
        if (func_002A86E8(&found, found, 0, 0) != 0) {
            return 0;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L06_0045A5B0);

INCLUDE_ASM("asm/levels", func_L06_0045BD38);

void func_L06_0045BFC0(unsigned char *a0)
{
    char *o = *(char **)(a0 + 0x68);

    switch (a0[0x20]) {
    case 0:
        *(unsigned short *)(a0 + 0x34) |= 0x800;
        func_002A7750((char *)a0, 1, -1);
        break;
    case 1:
        if (func_002A9640((char *)a0, 0x4010000, 0) != 0) {
            if (*(int *)o == 0) {
                func_002A7750((char *)a0, 2, -1);
            }
        }
        a0[0xA8] = 0xFF;
        break;
    case 2: {
        char tmp[16];

        func_00282D30(tmp, (char *)(a0 + 0xD0), -0.033333335f);
        func_L06_00457C00((char *)a0, 3, tmp, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f);
        func_002E5CF8(0, 0, (char *)a0);
        func_L00_00326110((char *)a0, 1, 0.0f);
        func_0029F338((char *)a0);
        break;
    }
    }
}

INCLUDE_ASM("asm/levels", func_L06_0045C0F0);

void func_L06_0045C350(unsigned char *a0)
{
    switch (a0[0x20]) {
    case 0:
        if (func_00273B70((char *)a0) != 0) {
            func_002A7750((char *)a0, 1, -1);
            *(unsigned short *)(a0 + 0x34) |= 0x2800;
        }
        break;
    case 1:
        func_00273B70((char *)a0);
        if (a0[0x23] < 2) {
            func_0029F338((char *)a0);
        } else {
            a0[0x23] -= 2;
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L06_0045C3E8);

INCLUDE_ASM("asm/levels", func_L06_0045C6D0);

INCLUDE_ASM("asm/levels", func_L06_004621C8);

INCLUDE_ASM("asm/levels", func_L06_004635E8);

INCLUDE_ASM("asm/levels", func_L06_00464900);

INCLUDE_ASM("asm/levels", func_L06_004649F0);

INCLUDE_ASM("asm/levels", func_L06_00464F40);

INCLUDE_ASM("asm/levels", func_L06_00471948);

void func_L06_00478548(float a, float b)
{
    char *g = D_L06_001B9890[0];
    char *p;
    char *v;

    if (*(short *)(g + 0x86) != 5) {
        p = func_00270280(5);
    } else {
        p = g;
    }
    v = *(char **)(p + 0x70) + 0xE0;
    *(float *)(v + 0x44) = a;
    *(float *)(v + 0x48) = b;
}

INCLUDE_ASM("asm/levels", func_L06_00498EF8);
