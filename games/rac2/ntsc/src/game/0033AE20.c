#include "include_asm.h"

typedef unsigned char u8;

typedef int s32;

extern void F624a0727_FUN_00336A78(char *target);

#ifndef RAC2_T_ENTRY_F624A0727
#define RAC2_T_ENTRY_F624A0727
typedef struct { char pad[8]; short n; void (*fn)(char *); } ENTRY_F624a0727;
#endif

typedef float f32;

extern void F8b17817c_FUN_003361A8(char *block);

extern void F8b17817c_FUN_00336968(char *block);

extern void F8b17817c_FUN_003366D0(char *block);

extern void Ffb202470_FUN_0033AB50(char *p);

/* Measured target: byte-identical across all 27 levels. */
void func_0033AE20(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x294) = a1; }

extern void func_00335E38(void *, float, float, float, float);
extern void func_003363C0(void *, float);

void func_0033AE28(char *object) {
    char *r = object + 12;
    char *q = object + 0x188;
    int i;
    for (i = 4; i >= 0; i--) {
        if (*(short *)q != 0) {
            float *position = *(float **)object;
            func_00335E38(r, position[0], position[1], 0.0f, 0.0f);
            func_003363C0(r, *(float *)(object + 0x274));
        }
        r += 76;
        q += 2;
    }
    if (*(int *)(object + 0x290) != 0) {
        func_00335E38(object + 0x198, (*(float **)object)[0] + *(float *)(object + 0x278), (*(float **)object)[1] + *(float *)(object + 0x27C), 0.0f, 0.0f);
        func_00335E38(object + 0x228, (*(float **)object)[0] + *(float *)(object + 0x288), (*(float **)object)[1] + *(float *)(object + 0x28C), 0.0f, 0.0f);
        func_00335E38(object + 0x1E0, (*(float **)object)[0] + *(float *)(object + 0x280), (*(float **)object)[1] + *(float *)(object + 0x284), 0.0f, 0.0f);
    }
}

void func_0033AF48(char *object)
{
    char *r = object + 12;
    char *q = object + 60;
    char *p = object + 392;
    int i;

    for (i = 4; i >= 0; i--) {
        if (*(short *)p != 0) {
            ENTRY_F624a0727 *t = *(ENTRY_F624a0727 **)q;
            t->fn(r + t->n);
        }
        r += 76;
        q += 76;
        p += 2;
    }
    if (*(int *)(object + 656) != 0) {
        F624a0727_FUN_00336A78(object + 408);
        if (*(int *)(object + 660) != 0) {
            if (*(int *)(object + 668) != 0)
                F624a0727_FUN_00336A78(object + 552);
            if (*(int *)(object + 664) != 0)
                F624a0727_FUN_00336A78(object + 480);
        }
    }
}

void func_0033B018(float **position, float x, float y) {
    (*position)[0] = x;
    (*position)[1] = y;
}

void func_0033B030(u8 *a0, f32 a1, f32 a2, f32 a3, f32 a4, f32 a5, f32 a6) {
    *(f32 *)(a0 + 0x278) = a1;
    *(f32 *)(a0 + 0x27C) = a2;
    *(f32 *)(a0 + 0x288) = a3;
    *(f32 *)(a0 + 0x28C) = a4;
    *(f32 *)(a0 + 0x280) = a5;
    *(f32 *)(a0 + 0x284) = a6;
}

void func_0033B050(int object, unsigned int a1, unsigned int a2, unsigned int a3)
{
    func_00336A38(object + 0x198, a1);
    func_00336A38(object + 0x228, a2);
    func_00336A38(object + 0x1E0, a3);
}

/* Measured target: byte-identical across all 27 levels. */
void func_0033B0A8(u8 *a0, f32 a1) { *(f32 *)(a0 + 0x274) = a1; }

INCLUDE_ASM("asm/game", func_0033B0B0);

char *func_0033B0B8(char *object)
{
    int index;
    char *block;

    F8b17817c_FUN_003361A8(object);
    F8b17817c_FUN_003361A8(object + 76);
    F8b17817c_FUN_003361A8(object + 152);
    block = object + 228;
    for (index = 2; index != -1; index--) {
        F8b17817c_FUN_003361A8(block);
        block += 76;
    }
    F8b17817c_FUN_00336968(object + 472);
    F8b17817c_FUN_00336968(object + 544);
    F8b17817c_FUN_00336968(object + 616);
    F8b17817c_FUN_003366D0(object + 688);
    F8b17817c_FUN_003366D0(object + 748);
    return object;
}

INCLUDE_ASM("asm/game", func_0033B160);

/* func_0033B170 is matched in 0033AE20-g8.c (-G8 small data). */

INCLUDE_ASM("asm/game", func_0033B538);

/* func_0033B540 is matched in 0033AE20-g8.c (-G8 small data). */

INCLUDE_ASM("asm/game", func_0033B910);

char *func_0033BD38(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033BD68);
