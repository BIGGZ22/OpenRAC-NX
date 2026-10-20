#include "include_asm.h"

extern int func_002ED688(char *);

extern void func_002EE678(char *, int);

extern int func_002A7820_1(int, int);

extern int func_001163B0(void);

extern int D_L05_00221D10[];

extern int D_L05_00221D28;

extern void func_L05_0037BB28(char *);

extern void func_L05_0037BD40(char *);

extern void func_L05_0037BC08(char *);

extern void func_L05_0037BF48(char *);

extern float func_332B50(float, float);

extern char *func_379CA0(char *);

extern void func_379FA0(char *, int, int, int, float, float, float, float, int, int, int);

extern int func_332AB8(int);

extern void func_3719A8(int, int, char *, int);

extern void func_306AD8(void *, void *, void *);

extern void func_306630(void *, void *, void *);

/* looks up an object, then sets flags, arms a stored value and calls one of two helper pairs before building a timer */
void func_L05_0037B6D0(char *obj)
{
    char *s;

    s = (char *)func_002ED688(obj);
    if (s == 0)
        return;
    if (*(int *)(s + 288) == 0)
        return;
    if (*(int *)(s + 284) != 0) {
        int *p = D_L05_00221D10;
        int one = 1;

        *(int *)(s + 280) = one;
        p[3] += one;
        if (func_L05_0037CEB8(obj) != 5) {
            func_002EE678(obj, 4);
            *(float *)(s + 120) = (func_001163B0() & 1) ? -1.0f : 1.0f;
            if (func_001163B0() & 1)
                *(float *)(s + 116) = -1.0f;
            else
                *(float *)(s + 116) = 1.0f;
            *(int *)(s + 252) = one;
            *(int *)(s + 112) = 0;
            *(int *)(s + 124) = 0;
        } else {
            func_002EE678(obj, 7);
        }
        *(int *)(s + 304) = func_002A7820_1(120, 500);
    } else {
        int *p = D_L05_00221D10;

        *(int *)(s + 280) = 1;
        p[3] += 1;
        if (func_L05_0037CEB8(obj) != 5)
            func_002EE678(obj, 4);
        else
            func_002EE678(obj, 7);
        *(int *)(s + 304) = func_002A7820_1(120, 500);
        *(int *)(s + 252) = 0;
    }
}


INCLUDE_ASM("asm/levels", func_L05_0037B948);

extern void func_3065F8(int);

struct S_L05_0037BB28 {
    char pad0[0x10];
    int f10;
    int f14;
    int f18;
    int f1C;
    int f20;
};

void func_L05_0037BB28(char *object)
{
    struct S_L05_0037BB28 *p = (struct S_L05_0037BB28 *)func_379CA0(object);

    if (p == 0)
        return;
    func_3065F8((int)p);
    p->f20 = -1;
    p->f1C = -1;
    p->f14 = 0;
    p->f18 = -1;
    p->f10 = 0;
}

INCLUDE_ASM("asm/levels", func_L05_0037BB78);

int func_L05_0037BB90(char *obj)
{
    int *p = (int *)func_002ED688(obj);

    if (p == 0)
        return 0;
    return p[5];
}

int func_L05_0037BBB8(char *obj)
{
    int *p = (int *)func_002ED688(obj);

    if (p == 0)
        return 0;
    if (p[5] == 0 && p[4] == 0)
        return 0;
    return 1;
}

INCLUDE_ASM("asm/levels", func_L05_0037BC08);

INCLUDE_ASM("asm/levels", func_L05_0037BD40);

void func_L05_0037BE98(char *obj)
{
    int *p = (int *)func_002ED688(obj);
    char *child;
    int *q;

    if (p == 0)
        return;
    if (func_L05_0037BB90(obj) != 0)
        func_L05_0037BD40(obj);
    child = (char *)p[4];
    q = (int *)func_002ED688(child);
    q[6] = q[6] - 1;
    func_L05_0037BB28(obj);
    if (q[6] < D_L05_00221D28)
        func_L05_0037BC08(child);
    else
        func_L05_0037BF48(child);
}

INCLUDE_ASM("asm/levels", func_L05_0037BF48);

INCLUDE_ASM("asm/levels", func_L05_0037C160);

void func_L05_0037CDD8(char *obj, int value)
{
    int *p = (int *)func_002ED688(obj);

    if (p != 0)
        *(int *)((char *)p + 0x114) = value;
}


int func_L05_0037CEB8(char *obj)
{
    int *p = (int *)func_002ED688(obj);

    if (p == 0)
        return 0;
    return *(int *)((char *)p + 0x114);
}

INCLUDE_ASM("asm/levels", func_L05_0037CEE0);


struct S_L05_002A1800 {
    char pad0[0xC];
    int f0C;
    int f10;
    int f14;
    char pad1[0x10];
};

extern struct S_L05_002A1800 D_L05_002A1800[];

extern int func_002ED8A0(int);

void func_L05_0037D6E8(void)
{
    int i;

    for (i = 0; i < 15; i++)
        D_L05_002A1800[i].f14 = D_L05_002A1800[i].f0C;
    D_L05_002A1800[7].f14 = D_L05_002A1800[7].f0C;
    D_L05_002A1800[13].f14 = D_L05_002A1800[13].f0C;
    i = func_002ED8A0(6);
    D_L05_002A1800[i].f14 = D_L05_002A1800[i].f10;
}

INCLUDE_ASM("asm/levels", func_L05_0037D768);

INCLUDE_ASM("asm/levels", func_L05_0037D7D8);

INCLUDE_ASM("asm/levels", func_L05_0037DA58);

INCLUDE_ASM("asm/levels", func_L05_003979B0);

INCLUDE_ASM("asm/levels", func_L05_00397B30);

INCLUDE_ASM("asm/levels", func_L05_00397F48);

INCLUDE_ASM("asm/levels", func_L05_00398018);

INCLUDE_ASM("asm/levels", func_L05_00398378);

INCLUDE_ASM("asm/levels", func_L05_00398618);

INCLUDE_ASM("asm/levels", func_L05_00398A38);

INCLUDE_ASM("asm/levels", func_L05_00398F18);

INCLUDE_ASM("asm/levels", func_L05_00399268);

INCLUDE_ASM("asm/levels", func_L05_003CD1A8);

INCLUDE_ASM("asm/levels", func_L05_003CD280);

void func_L05_003CD4C0(char *object)
{
    char *state = *(char **)(object + 0x68);
    float *f = (float *)state;
    int *w;
    int i;
    int j;
    int v;

    for (i = 3; i >= 0; i--)
        *f++ = (float)func_332AB8(0xFF);
    *(float *)(state + 0x10) = -1.0f;
    *(float *)(state + 0x14) = -2.25f;
    *(float *)(state + 0x18) = 1.25f;
    *(float *)(state + 0x1C) = 2.5f;
    j = 3;
    w = (int *)(state + 0x2C);
    v = 0xFF;
    for (; j >= 0; j--) {
        *w = v;
        w = (int *)((char *)w - 4);
        v -= 0x40;
    }
    *(float *)(state + 0x30) = 2.8f;
    *(float *)(state + 0x34) = 3.5f;
    *(float *)(state + 0x3C) = 2.8f;
    *(float *)(state + 0x38) = 3.5f;
}

INCLUDE_ASM("asm/levels", func_L05_003CD598);

INCLUDE_ASM("asm/levels", func_L05_003CEFA0);

INCLUDE_ASM("asm/levels", func_L05_003CF040);

INCLUDE_ASM("asm/levels", func_L05_003CF350);

INCLUDE_ASM("asm/levels", func_L05_003CFAB8);

INCLUDE_ASM("asm/levels", func_L05_003CFB90);

int func_L05_003CFC28(int a0)
{
    if (a0 == 0x971)
        return 4;
    return a0 - 0x953;
}

INCLUDE_ASM("asm/levels", func_L05_003CFC48);

INCLUDE_ASM("asm/levels", func_L05_003CFCF0);

INCLUDE_ASM("asm/levels", func_L05_003CFF90);

INCLUDE_ASM("asm/levels", func_L05_003D02C8);

INCLUDE_ASM("asm/levels", func_L05_003D0320);

INCLUDE_ASM("asm/levels", func_L05_003D0538);

INCLUDE_ASM("asm/levels", func_L05_003D0728);

void func_L05_003D08C0(float *a0, char *a1)
{
    a0[0] = func_332B50(-1.0f, 1.0f);
    a0[1] = func_332B50(-1.0f, 1.0f);
    a0[2] = func_332B50(-1.0f, 1.0f);
    a0[3] = 1.0f;
    func_306AD8(a0, a0, a1);
    func_306630(a0, a0, a1 + 0x30);
}

INCLUDE_ASM("asm/levels", func_L05_003D0960);

INCLUDE_ASM("asm/levels", func_L05_003D0B20);

extern void func_002AFFD8(char *a0, char *a1, int a2);

extern float *func_002AB1F0(char *a0);

extern void func_00282D30(void *a0, void *a1, float a2);

extern void func_00282CC0(void *a0, void *a1, void *a2);

extern void func_L05_003D1148(char *a0, char *a1, void *a2, char *a3, void *a4,
                              float a5, float a6, float a7);

/* Builds the pair of vectors the timer wants: the object's own matrix, scaled by the value the
 * looked-up sub-object holds, and that scaled matrix combined with the sub-object's table. */
void func_L05_003D1058(char *a0, char *a1, char *a2, char *a3, float f0, float f1, float f2)
{
    float v0[4];
    float v1[4];
    float v2[4];
    float *p;

    if (a2 == 0)
        return;
    if (*(unsigned char *)(a2 + 0x20) == 0xFE)
        return;
    if (*(unsigned char *)(a2 + 0x20) == 0xFD)
        return;
    func_002AFFD8(a0, (char *)v0, 1);
    p = func_002AB1F0(a2);
    func_00282D30(v1, v0, *(float *)((char *)p + 0x10));
    func_00282CC0(v2, (float *)(a2 + 0x10), v1);
    func_L05_003D1148(a0, a1, v2, a3, v0, f0, f1, f2);
}

INCLUDE_ASM("asm/levels", func_L05_003D1148);

INCLUDE_ASM("asm/levels", func_L05_003D1368);

INCLUDE_ASM("asm/levels", func_L05_003D15E8);

INCLUDE_ASM("asm/levels", func_L05_003D18E0);

/* Spawns the object's effect after three guards, then runs the timer. */
void func_L05_003D19F0(char *object)
{
    char *s1 = *(char **)(object + 0x68);
    char *r;

    r = func_379CA0(object);
    if (r == 0)
        return;
    if (*(int *)(r + 0x120) == 0)
        return;
    if (*(int *)(s1 + 0x268) != 0)
        return;
    func_379FA0(object, 0x60000090, 1, 0, 0.6f, 2.0f, 1.0f, 1.3f, 0, -1, 0);
    if (func_332AB8(3) == 0)
        func_3719A8(0, 0, object, 0x97C);
    *(int *)(s1 + 0x268) = 0x3C;
}

INCLUDE_ASM("asm/levels", func_L05_003D1AB8);

INCLUDE_ASM("asm/levels", func_L05_003D1E20);

INCLUDE_ASM("asm/levels", func_L05_003D1E60);

INCLUDE_ASM("asm/levels", func_L05_003D2048);

INCLUDE_ASM("asm/levels", func_L05_003D2618);
