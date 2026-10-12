#include "include_asm.h"

extern void func_00282CF0_9(char *local, char *first, char *second);

extern void func_00282CC0_15(char *target, char *first, char *local);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

extern int func_00282978_5(char *handle);

extern void func_0029F338_2(char *object);

INCLUDE_ASM("asm/levels", func_L00_00412928);

INCLUDE_ASM("asm/levels", func_L00_00412FF8);

INCLUDE_ASM("asm/levels", func_L00_00413128);

INCLUDE_ASM("asm/levels", func_L00_00413298);

INCLUDE_ASM("asm/levels", func_L00_00413358);

INCLUDE_ASM("asm/levels", func_L00_00413870);

INCLUDE_ASM("asm/levels", func_L00_00413918);

void func_L00_00414018(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00414020);

INCLUDE_ASM("asm/levels", func_L00_00414350);

INCLUDE_ASM("asm/levels", func_L00_004143D0);

INCLUDE_ASM("asm/levels", func_L00_00414540);

INCLUDE_ASM("asm/levels", func_L00_00415210);

INCLUDE_ASM("asm/levels", func_L00_00415740);

INCLUDE_ASM("asm/levels", func_L00_00415A38);

INCLUDE_ASM("asm/levels", func_L00_00415AA0);

INCLUDE_ASM("asm/levels", func_L00_00415DE8);

INCLUDE_ASM("asm/levels", func_L00_00416A38);

INCLUDE_ASM("asm/levels", func_L00_00416F68);

INCLUDE_ASM("asm/levels", func_L00_004170A8);

INCLUDE_ASM("asm/levels", func_L00_00417218);

INCLUDE_ASM("asm/levels", func_L00_004173B0);

INCLUDE_ASM("asm/levels", func_L00_00417700);

INCLUDE_ASM("asm/levels", func_L00_00417868);

INCLUDE_ASM("asm/levels", func_L00_00417A40);

INCLUDE_ASM("asm/levels", func_L00_00417D70);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_00418140(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_004181A8(char *p)
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
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L00_00418230);

INCLUDE_ASM("asm/levels", func_L00_00418730);

/* Tests the handle cached in the object and, while the handle reports itself already released, drops the next sub-handle, otherwise runs the object's destroy helper. */
void func_L00_00418A28(char *object)
{
    char *handle = *(char **)(object + 104);

    if (func_00282978_5(handle) == 0)
        func_00282978_5(handle + 4);
    else
        func_0029F338_2(object);
}

INCLUDE_ASM("asm/levels", func_L00_00418A80);

INCLUDE_ASM("asm/levels", func_L00_004195C8);

INCLUDE_ASM("asm/levels", func_L00_00419668);

INCLUDE_ASM("asm/levels", func_L00_00419A80);

INCLUDE_ASM("asm/levels", func_L00_00419B58);

INCLUDE_ASM("asm/levels", func_L00_00419C98);

INCLUDE_ASM("asm/levels", func_L00_0041A490);

INCLUDE_ASM("asm/levels", func_L00_0041A500);

INCLUDE_ASM("asm/levels", func_L00_0041A5C0);
