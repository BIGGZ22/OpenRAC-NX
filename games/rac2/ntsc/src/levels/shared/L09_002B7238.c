#include "include_asm.h"

/* index -> entry of a per-level table; a live entry's first word is a one-based id. */
extern unsigned int D_L09_001FE6C0[];
extern unsigned int D_L09_001FB560[];
extern char *D_L09_001B285C;

INCLUDE_ASM("asm/levels", func_L09_002B7238);

INCLUDE_ASM("asm/levels", func_L09_00325C58);


extern int func_002AD980(void);
extern int D_0018C0B0;

INCLUDE_ASM("asm/levels", func_L09_00326E90);



extern int func_002B6120(char *object, unsigned int entry);

int func_L09_00329028(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return 0;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return 0;
    return func_002B6120(object, entry);
}



extern int func_002B6228(char *object, unsigned int entry);

int func_L09_00329178(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return 0;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return 0;
    return func_002B6228(object, entry);
}


INCLUDE_ASM("asm/levels", func_L09_003364E0);


INCLUDE_ASM("asm/levels", func_L09_0033E658);


extern int func_L06_00382850(char *object, unsigned int entry);

int func_L09_0033E690(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return -1;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return -1;
    return func_L06_00382850(object, entry);
}



extern float func_L01_003377C8(void *entry);

float func_L09_0033E7A0(int index)
{
    unsigned int entry;

    if (index == -1)
        return -1.0f;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return -1.0f;
    return func_L01_003377C8((void *)entry);
}



extern int func_L01_00337888(char *object, unsigned int entry);

int func_L09_0033E8B0(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return -1;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return -1;
    return func_L01_00337888(object, entry);
}


INCLUDE_ASM("asm/levels", func_L09_0037FCE8);

INCLUDE_ASM("asm/levels", func_L09_003806A0);

INCLUDE_ASM("asm/levels", func_L09_003A01D8);

INCLUDE_ASM("asm/levels", func_L09_003A06B0);

INCLUDE_ASM("asm/levels", func_L09_003AE8C8);

INCLUDE_ASM("asm/levels", func_L09_003AEA88);


extern void func_00282CC0(char *first, char *second, char *third);
extern void func_00282DD8(char *first, char *second, char *third);
extern void func_00282F20(char *dst, char *src, float x);
extern float func_002A7940(void);

void func_L09_003AEE10(char *object)
{
    char *p = *(char **)(object + 104);
    char *q = object + 0x10;

    func_00282CC0(p, q, p + 0x30);
    func_00282DD8(p, p, q);
    func_00282F20(p, p, 0.25f);
    *(float *)(p + 0x5C) = func_002A7940();
}


INCLUDE_ASM("asm/levels", func_L09_003AEE80);

INCLUDE_ASM("asm/levels", func_L09_003AF060);

INCLUDE_ASM("asm/levels", func_L09_003AF508);

INCLUDE_ASM("asm/levels", func_L09_003AF6B0);

INCLUDE_ASM("asm/levels", func_L09_003AF9E8);

INCLUDE_ASM("asm/levels", func_L09_003B6550);

INCLUDE_ASM("asm/levels", func_L09_003E99E0);

INCLUDE_ASM("asm/levels", func_L09_003E9AA0);

INCLUDE_ASM("asm/levels", func_L09_003E9E60);

INCLUDE_ASM("asm/levels", func_L09_003EA058);

INCLUDE_ASM("asm/levels", func_L09_003EA108);


extern int func_L06_00382850_1(char *object, unsigned int entry, float value);
extern int func_002B6228_1(char *object, unsigned int entry, int value, int flag);

INCLUDE_ASM("asm/levels", func_L09_003EA2D0);


INCLUDE_ASM("asm/levels", func_L09_003EA360);

INCLUDE_ASM("asm/levels", func_L09_003EA798);


extern void func_00282978(void *p);
extern void func_002829A8(void *p);

void func_L09_003EA9D0(char *object)
{
    unsigned short count = *(unsigned short *)(object + 0x96);
    char *p = *(char **)(object + 104);

    if (count <= 0xFFFE)
        *(unsigned short *)(object + 0x96) = count + 1;
    func_00282978(p + 0x36A);
    func_002829A8(p + 0x368);
}


INCLUDE_ASM("asm/levels", func_L09_003EAA18);

INCLUDE_ASM("asm/levels", func_L09_003EAB20);

INCLUDE_ASM("asm/levels", func_L09_003EAF60);

INCLUDE_ASM("asm/levels", func_L09_003F65E0);


extern float func_00282E48(char *a, char *b);
extern void func_002A93B0(void *object);
extern char D_L09_001B9A00[];

void func_L09_003F7158(char *object)
{
    if (*(unsigned char *)(object + 0x31) != 0) {
        if (func_00282E48(object + 0x10, D_L09_001B9A00) < 24.0f) {
            func_002A93B0(object);
            *(unsigned char *)(object + 0x6F) = 0x16;
        }
    }
}


INCLUDE_ASM("asm/levels", func_L09_003F71C8);

INCLUDE_ASM("asm/levels", func_L09_003F72A8);

INCLUDE_ASM("asm/levels", func_L09_003F7370);


extern void func_00282948(void *a0);

void func_L09_003F73E8(char *object)
{
    char *p = *(char **)(object + 104);

    if (*(unsigned char *)(object + 33) != 0xFF && *(unsigned char *)(p + 0x2EB) != 0) {
        func_00282948(*(void **)(p + 0x304));
    }
    *(unsigned char *)(p + 0x2EB) = 0;
}


void func_L09_003F7430(char *object)
{
    char *p = *(char **)(object + 104);
    char *q;

    if (*(unsigned char *)(object + 33) == 0xFF)
        return;
    if (*(unsigned char *)(p + 0x2EB) != 0)
        return;
    q = *(char **)(p + 0x304);
    (*(int *)q)++;
    *(unsigned char *)(p + 0x2EB) = 1;
}


void func_L09_003F7468(char *object)
{
    char *parent;

    char *p;

    p = *(char **)(object + 104);
    *(int *)(object + 152) = 0;
    parent = *(char **)(p + 0x2F8);
    if (parent == 0)
        return;
    if (*(short *)(parent + 170) != 3342)
        return;
    if (*(unsigned char *)(parent + 32) == 254)
        return;
    if (*(unsigned char *)(parent + 32) != 253)
        *(int *)(parent + 152) = 0;
}

/* shared native body of family cd6f3d9eca5f8edd */
/* attempt 1 - shape read straight off the disassembly:
   save object's word 152 from its owner's field 16, then repeat for a second
   object found through two pointer hops, skipping the sentinel byte values. */

void func_L09_003F74A8(char *p)
{
    char *q;
    char *r;
    char *s;
    int v;

    if (p == 0)
        return;
    if (*(unsigned char *)(p + 32) == 254)
        return;
    if (*(unsigned char *)(p + 32) == 253)
        return;

    q = *(char **)(p + 36);
    r = *(char **)(p + 104);
    *(int *)(p + 152) = *(int *)(q + 16);

    s = *(char **)(r + 760);
    if (s == 0)
        return;
    if (*(short *)(s + 170) != 3342)
        return;
    if (*(unsigned char *)(s + 32) == 254)
        return;
    if (*(unsigned char *)(s + 32) == 253)
        return;

    v = *(int *)(s + 36);
    *(int *)(s + 152) = *(int *)(v + 16);
}

INCLUDE_ASM("asm/levels", func_L09_003F7518);


extern void func_002A04B8(char *object, int *out0, int *out1, int *out2);
extern void func_002A0488(char *object, int a, int b, int c);

void func_L09_003F7788(char *object, char *target)
{
    int out0, out1, out2;

    func_002A04B8(object, &out0, &out1, &out2);
    func_002A0488(target, out0, out1, out2);
}


extern void func_0029F338(void *entry);

void func_L09_003F77D0(char *object)
{
    char *descriptor = *(char **)(object + 0x68);
    char *slots;
    int i;

    if (*(int *)(descriptor + 0x2F8) != 0) {
        func_0029F338(*(void **)(descriptor + 0x2F8));
        *(int *)(descriptor + 0x2F8) = 0;
    }
    slots = descriptor + 0x2F0;
    for (i = 0; i < 2; ++i) {
        if (*(int *)(slots + i * 4) != 0) {
            func_0029F338(*(void **)(slots + i * 4));
            *(int *)(slots + i * 4) = 0;
        }
    }
}


INCLUDE_ASM("asm/levels", func_L09_003F7840);

INCLUDE_ASM("asm/levels", func_L09_003F7A88);

INCLUDE_ASM("asm/levels", func_L09_003F7B28);


extern int func_L09_0033E690_1(char *object, int index, float value);
extern int func_L09_00329178_1(char *object, int index, int value, int flag);

INCLUDE_ASM("asm/levels", func_L09_003F7C50);


INCLUDE_ASM("asm/levels", func_L09_003F7CE0);

INCLUDE_ASM("asm/levels", func_L09_003F8598);


INCLUDE_ASM("asm/levels", func_L09_003F87D8);



extern void ref_L09_003F8858_00(char *object, int value);

void func_L09_003F8858(char *object)
{
    char *p = *(char **)(object + 104);

    ref_L09_003F8858_00(object, *(unsigned char *)(object + 33));
    ref_L09_003F8858_00(object, *(int *)(p + 0x308));
}

INCLUDE_ASM("asm/levels", func_L09_003F8898);

INCLUDE_ASM("asm/levels", func_L09_003F8960);

INCLUDE_ASM("asm/levels", func_L09_003F8AE0);

INCLUDE_ASM("asm/levels", func_L09_003F9040);

INCLUDE_ASM("asm/levels", func_L09_003F9108);
