#include "include_asm.h"

extern void func_00282CF0_9(char *local, char *first, char *second);

extern void func_00282CC0_15(char *target, char *first, char *local);

extern void func_3251D0();

extern int func_31E7A8();

extern float func_2EE238();

extern void func_2EE178(void *, void *, float);

extern void func_2EE108();

extern void func_2EE138();

extern void func_2EE3D0(void *, void *, float);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

INCLUDE_ASM("asm/levels", func_L00_003A8E28);

INCLUDE_ASM("asm/levels", func_L00_003A9000);

INCLUDE_ASM("asm/levels", func_L00_003A91A0);

INCLUDE_ASM("asm/levels", func_L00_003A9798);

INCLUDE_ASM("asm/levels", func_L00_003A98E8);

INCLUDE_ASM("asm/levels", func_L00_003A9C58);

INCLUDE_ASM("asm/levels", func_L00_003AA4C8);

INCLUDE_ASM("asm/levels", func_L00_003AA708);

INCLUDE_ASM("asm/levels", func_L00_003AB388);

/* Applies a state record to a 3D point when the record carries a live tag. */
void func_L00_003AB618(char *a0, char *a1, char *a2)
{
    if (a2 != 0) {
        unsigned char t = *(unsigned char *)(a2 + 0x20);

        if (t != 0xFE)
            if (t != 0xFD) {
                char b0[16];
                char b1[16];
                char b2[16];
                float f;

                func_3251D0(a0, b0, 1);
                f = *(float *)(func_31E7A8(a2) + 0x10);
                func_2EE178(b1, b0, f);
                func_2EE108(b2, a2 + 0x10, b1);
                f = func_2EE238(a1);
                func_2EE138(a1, b2, a0);
                func_2EE3D0(a1, a1, f);
            }
    }
}

INCLUDE_ASM("asm/levels", func_L00_003AB6F0);

INCLUDE_ASM("asm/levels", func_L00_003AB8B0);

INCLUDE_ASM("asm/levels", func_L00_003AB8B8);

INCLUDE_ASM("asm/levels", func_L00_003AB9C0);

INCLUDE_ASM("asm/levels", func_L00_003ABEC0);

INCLUDE_ASM("asm/levels", func_L00_003AC8E8);

INCLUDE_ASM("asm/levels", func_L00_003ACA60);

INCLUDE_ASM("asm/levels", func_L00_003ACC88);


INCLUDE_ASM("asm/levels", func_L00_003AD7C8);

INCLUDE_ASM("asm/levels", func_L00_003ADA30);

INCLUDE_ASM("asm/levels", func_L00_003ADCD8);

INCLUDE_ASM("asm/levels", func_L00_003ADEA0);

INCLUDE_ASM("asm/levels", func_L00_003AE550);

INCLUDE_ASM("asm/levels", func_L00_003AE900);

INCLUDE_ASM("asm/levels", func_L00_003B0A28);

INCLUDE_ASM("asm/levels", func_L00_003B0AB0);

INCLUDE_ASM("asm/levels", func_L00_003B0B20);

INCLUDE_ASM("asm/levels", func_L00_003B0CE8);

INCLUDE_ASM("asm/levels", func_L00_003B0DA0);

INCLUDE_ASM("asm/levels", func_L00_003B1270);

INCLUDE_ASM("asm/levels", func_L00_003B14D0);

/* A variadic forwarder whose whole retail body is the register save area the EE compiler
   emits for a variadic function: four named integer arguments consume $a0-$a3, so the
   variadic ones are saved out of $t0-$t3 and $f12/$f18. Nothing else is left of it. */
void func_L00_003B1A08(int a0, int a1, int a2, int a3, ...)
{
}

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003B2900(char *p)
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

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_003B2B60(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/levels", func_L00_003B3090);
