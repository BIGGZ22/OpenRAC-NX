#include "include_asm.h"
#include "common.h"

extern void func_3065F8(char *block);

extern void func_306630(char *dst, char *src, char *src2);

extern void func_306660(char *dst, char *src, char *src2);

extern void func_3066A0(char *dst, char *src, float value);

extern void func_306AD8(char *dst, char *src, char *object);

extern char *func_328508(int id);

extern int func_332AB8(int kind);

extern float func_332B50(float a, float b);

extern float func_332C18(void);

extern int func_334CB8(char *object, char *state, int value);

extern void func_32AF08(char *object);

extern void func_32E728(char *object, int flag, int *mode, char *out);

extern void func_33B330(char *dst, float a, float b, float c);

extern void func_3719A8(int a0, int a1, char *object, int a3);

extern char *func_379CA0(char *object);

extern void func_379FA0(char *object, int a1, int a2, int a3, float f0, float f1, float f2,
                        float f3, int a8, int a9, int a10);

extern void func_37AEC0(char *object, int flag);

extern void func_L05_00416E08(char *object);

extern char D_L05_002659E0[];

extern int D_L05_00221D10[];

extern char *D_L05_001B2614;

INCLUDE_ASM("asm/levels", func_L05_00314E28);

INCLUDE_ASM("asm/levels", func_L05_00367340);

INCLUDE_ASM("asm/levels", func_L05_003A3CE0);

INCLUDE_ASM("asm/levels", func_L05_003A40F8);

INCLUDE_ASM("asm/levels", func_L05_003A96D8);

INCLUDE_ASM("asm/levels", func_L05_003D8E98);

INCLUDE_ASM("asm/levels", func_L05_00414F90);

INCLUDE_ASM("asm/levels", func_L05_004155A0);

INCLUDE_ASM("asm/levels", func_L05_004157B0);

/* Runs the object's pending effect when three guards clear, then marks the run. */
void func_L05_004158B0(char *object)
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
    if (*(unsigned char *)(object + 0x20) == 1)
        func_379FA0(object, 0x60800000, 1, 0, 0.75f, 5.0f, 1.0f, 1.0f, 0, -1, 0);
    *(int *)(s1 + 0x268) = 0x14;
}

INCLUDE_ASM("asm/levels", func_L05_00415958);

INCLUDE_ASM("asm/levels", func_L05_00415B38);

/* Clears one int field of each of 128 strided records. */
INCLUDE_ASM("asm/levels", func_L05_00415D70);

INCLUDE_ASM("asm/levels", func_L05_00415DA8);

/* Clears one of two state words on the object's sub-record, by mode. */
void func_L05_00415EE8(int object, int mode)
{
    int s = *(int *)(object + 0x68);

    switch (mode) {
    case 0:
        *(int *)(s + 0x26C) = 0;
        break;
    case 1:
        *(int *)(s + 0x270) = 0;
        break;
    }
}

/* Spawns the object's effect after three guards, then runs the follow-up by kind. */
void func_L05_00415F18(char *object)
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
    func_379FA0(object, 0x60408020, 1, 0, 0.75f, 5.0f, 1.0f, 1.0f, 0, -1, 0);
    if (func_332AB8(3) != 0)
        func_3719A8(0, 0, object, 0xF76);
    *(int *)(s1 + 0x268) = 0xF;
}

INCLUDE_ASM("asm/levels", func_L05_00415FD0);

INCLUDE_ASM("asm/levels", func_L05_004161B0);

INCLUDE_ASM("asm/levels", func_L05_00416468);

INCLUDE_ASM("asm/levels", func_L05_00416698);

INCLUDE_ASM("asm/levels", func_L05_00416918);

char *func_L05_00416D68(char *a0, int a1)
{
    char *obj = func_328508(0xF81);

    if (obj != 0) {
        char *sub = *(char **)(obj + 0x68);

        *(unsigned char *)(obj + 0x30) = 0xFF;
        *(unsigned short *)(obj + 0x32) = 0x1FF;
        *(unsigned short *)(obj + 0x34) = *(unsigned short *)(obj + 0x34) | 0x100;
        *(unsigned char *)(obj + 0x20) = 0;
        *(char **)(sub + 0x50) = a0;
        *(int *)(sub + 0x54) = a1;
        func_3065F8(sub + 0x30);
        func_3065F8(sub + 0x40);
        func_L05_00416E08(obj);
        func_32AF08(obj);
    }
    return obj;
}

void func_L05_00416E08(char *object)
{
    char *sub = *(char **)(object + 0x68);
    char *src = *(char **)(sub + 0x50);

    qcopy(object + 0x10, src + 0x10);
    switch (*(int *)(sub + 0x54)) {
    case 0:
        qcopy(object + 0xC0, src + 0xC0);
        qcopy(object + 0xD0, src + 0xD0);
        break;
    case 1:
        qcopy(object + 0xC0, src + 0xC0);
        func_3066A0(object + 0xD0, src + 0xD0, -1.0f);
        break;
    default:
        return;
    }
    qcopy(object + 0xE0, src + 0xE0);
}

INCLUDE_ASM("asm/levels", func_L05_00416EC0);

INCLUDE_ASM("asm/levels", func_L05_00416F80);

int func_L05_00417088(char *object)
{
    int r = func_334CB8(object, (char *)0x230000, 0);

    *(unsigned char *)(object + 0xA8) = 0xFF;
    if (r != 0)
        *(unsigned char *)(object + 0x20) = 4;
}

INCLUDE_ASM("asm/levels", func_L05_0041C860);

INCLUDE_ASM("asm/levels", func_L05_0041CA70);

INCLUDE_ASM("asm/levels", func_L05_0041CB68);

INCLUDE_ASM("asm/levels", func_L05_0041CE00);

INCLUDE_ASM("asm/levels", func_L05_0041CFE8);

/* shared native body of family 5a97ab89858c */
void func_L05_0041D1B0(char *p)
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

INCLUDE_ASM("asm/levels", func_L05_0041D218);

INCLUDE_ASM("asm/levels", func_L05_0041DA20);

INCLUDE_ASM("asm/levels", func_L05_0041DF58);

INCLUDE_ASM("asm/levels", func_L05_0041EF00);

INCLUDE_ASM("asm/levels", func_L05_0041FC60);

INCLUDE_ASM("asm/levels", func_L05_0041FD48);

INCLUDE_ASM("asm/levels", func_L05_0041FE98);

INCLUDE_ASM("asm/levels", func_L05_0041FFF8);
