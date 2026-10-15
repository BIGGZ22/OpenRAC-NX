#include "include_asm.h"
#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

INCLUDE_ASM("asm/levels", func_L05_003D33E8);

INCLUDE_ASM("asm/levels", func_L05_003FE430);

/* shared native body of family 5a97ab89858c */
void func_L05_00416630(char *p)
{
    *(long long *)(p + 112) = 0;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L05_004170C8);

INCLUDE_ASM("asm/levels", func_L05_00417448);

INCLUDE_ASM("asm/levels", func_L05_00417550);

INCLUDE_ASM("asm/levels", func_L05_00417D48);

INCLUDE_ASM("asm/levels", func_L05_00418288);

INCLUDE_ASM("asm/levels", func_L05_00418440);

INCLUDE_ASM("asm/levels", func_L05_00418508);

INCLUDE_ASM("asm/levels", func_L05_004186D8);

INCLUDE_ASM("asm/levels", func_L05_00418D68);

INCLUDE_ASM("asm/levels", func_L05_00418E90);

INCLUDE_ASM("asm/levels", func_L05_00419080);

INCLUDE_ASM("asm/levels", func_L05_00419370);

INCLUDE_ASM("asm/levels", func_L05_00419888);

INCLUDE_ASM("asm/levels", func_L05_00419CC8);

INCLUDE_ASM("asm/levels", func_L05_00419F10);

void func_L05_0041A218(void)
{
    char buf[0x20];
}

INCLUDE_ASM("asm/levels", func_L05_0041A228);



INCLUDE_ASM("asm/levels", func_L05_0041C118);

INCLUDE_ASM("asm/levels", func_L05_0041C1F0);

extern int func_332AB8(int kind);

void func_L05_0041C410(char *object)
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
    *(float *)(state + 0x30) = 3.0f;
    *(float *)(state + 0x34) = 3.7f;
    *(float *)(state + 0x3C) = 3.0f;
    *(float *)(state + 0x38) = 3.7f;
}

INCLUDE_ASM("asm/levels", func_L05_0041C4E8);

INCLUDE_ASM("asm/levels", func_L05_0041C708);

INCLUDE_ASM("asm/levels", func_L05_0041C7E8);

/* func_L05_0041D498 is defined in L05_003D33E8-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L05_0041D570);

void func_L05_0041D728(char *object)
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

INCLUDE_ASM("asm/levels", func_L05_0041D800);

INCLUDE_ASM("asm/levels", func_L05_0041ED18);

INCLUDE_ASM("asm/levels", func_L05_00421860);

INCLUDE_ASM("asm/levels", func_L05_00421908);

extern void func_L00_002E6EF8(void *, void *);

extern int ref_L05_00424780_00;

void func_L05_00424780(char *object)
{
    char *state = *(char **)(object + 0x68);

    if (*(int *)(state + 0xC) != 0)
        func_L00_002E6EF8(&ref_L05_00424780_00, object);
    *(unsigned short *)(object + 0x34) |= 0x41;
}

INCLUDE_ASM("asm/levels", func_L05_004247C8);

void func_L05_00424970(char *object, int value)
{
    char *state = *(char **)(object + 0x68);

    *(int *)(state + 0x8) = value;
}

void func_L05_00424980(char *object, int value)
{
    char *state = *(char **)(object + 0x68);

    *(int *)(state + 0xC) = value;
}

INCLUDE_ASM("asm/levels", func_L05_00424990);

extern void func_0029F338(char *);

extern int func_00282978(char *);

extern int ref_L05_00424C88_00;

extern void ref_L05_00424C88_01(void *, void *);

void func_L05_00424C88(char *object)
{
    char *state = *(char **)(object + 0x68);

    if (func_00282978(state) == 0) {
        func_00282978(state + 4);
        ref_L05_00424C88_01(&ref_L05_00424C88_00, object);
    } else {
        func_0029F338(object);
    }
}

INCLUDE_ASM("asm/levels", func_L05_00424CF0);

INCLUDE_ASM("asm/levels", func_L05_00425058);

/* func_L05_00425FE0 is defined in L05_003D33E8-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L05_004260B8);

void func_L05_00426270(char *object)
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
    *(float *)(state + 0x30) = 2.4f;
    *(float *)(state + 0x34) = 3.0f;
    *(float *)(state + 0x3C) = 2.4f;
    *(float *)(state + 0x38) = 3.0f;
}

INCLUDE_ASM("asm/levels", func_L05_00426348);

INCLUDE_ASM("asm/levels", func_L05_00426568);

INCLUDE_ASM("asm/levels", func_L05_00426658);

void func_L05_00426810(char *object)
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
    *(float *)(state + 0x30) = 2.0f;
    *(float *)(state + 0x34) = 2.5f;
    *(float *)(state + 0x3C) = 2.0f;
    *(float *)(state + 0x38) = 2.5f;
}

INCLUDE_ASM("asm/levels", func_L05_004268E8);

/* func_L05_00426B08 is defined in L05_003D33E8-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L05_00426BE0);

void func_L05_00426D98(char *object)
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
    *(float *)(state + 0x30) = 2.5f;
    *(float *)(state + 0x34) = 3.2f;
    *(float *)(state + 0x3C) = 2.5f;
    *(float *)(state + 0x38) = 3.2f;
}

INCLUDE_ASM("asm/levels", func_L05_00426E70);

INCLUDE_ASM("asm/levels", func_L05_00427090);

INCLUDE_ASM("asm/levels", func_L05_00427238);

INCLUDE_ASM("asm/levels", func_L05_004276E8);

INCLUDE_ASM("asm/levels", func_L05_00427968);

INCLUDE_ASM("asm/levels", func_L05_00427988);

INCLUDE_ASM("asm/levels", func_L05_00427B20);

INCLUDE_ASM("asm/levels", func_L05_00427E10);

INCLUDE_ASM("asm/levels", func_L05_00427FA8);

INCLUDE_ASM("asm/levels", func_L05_00427FC8);
