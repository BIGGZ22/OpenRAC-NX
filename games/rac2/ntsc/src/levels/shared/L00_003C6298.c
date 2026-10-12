#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner {
    unsigned char prefix_00[0x68];
    void *context_68;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner;

typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext {
    unsigned char prefix_00[0x10];
    float vector_10[4];
    unsigned int field_20;
    unsigned int active_24;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext;

extern void func_002FDBD8_3(float scale, Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context,
                                    float *vector, int mode);

extern void func_00282D30_2(char *dst, char *src, float scale);

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

INCLUDE_ASM("asm/levels", func_L00_003C6298);

INCLUDE_ASM("asm/levels", func_L00_003C6A88);

INCLUDE_ASM("asm/levels", func_L00_003C6B98);

INCLUDE_ASM("asm/levels", func_L00_003C79F8);

INCLUDE_ASM("asm/levels", func_L00_003C7DF0);

INCLUDE_ASM("asm/levels", func_L00_003C83E0);

INCLUDE_ASM("asm/levels", func_L00_003C8608);

INCLUDE_ASM("asm/levels", func_L00_003C8828);

/* When the context flag at offset 0x24 is nonzero, pass scale 1.0, the context, its vector at offset 0x10 and mode zero to the bound helper; original game role is unknown. */
void func_L00_003C8A58(Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context = owner->context_68;
    if (context->active_24 != 0) {
        func_002FDBD8_3(1.0f, context, context->vector_10, 0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003C8A90);

INCLUDE_ASM("asm/levels", func_L00_003C94D8);

INCLUDE_ASM("asm/levels", func_L00_003C9978);

INCLUDE_ASM("asm/levels", func_L00_003CA470);

INCLUDE_ASM("asm/levels", func_L00_003CA650);

INCLUDE_ASM("asm/levels", func_L00_003CA688);

INCLUDE_ASM("asm/levels", func_L00_003CA730);

INCLUDE_ASM("asm/levels", func_L00_003CADA0);

INCLUDE_ASM("asm/levels", func_L00_003CB538);

INCLUDE_ASM("asm/levels", func_L00_003CB980);

INCLUDE_ASM("asm/levels", func_L00_003CBBD0);

INCLUDE_ASM("asm/levels", func_L00_003CBBE0);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003CBC80(char *p)
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

INCLUDE_ASM("asm/levels", func_L00_003CBD08);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L00_003CBE28(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L00_003CC030);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_003CC6D8(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

void func_L00_003CD158(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_003CEC50);

INCLUDE_ASM("asm/levels", func_L00_003CED50);

INCLUDE_ASM("asm/levels", func_L00_003D1258);

INCLUDE_ASM("asm/levels", func_L00_003D1B48);

INCLUDE_ASM("asm/levels", func_L00_003D1B50);

INCLUDE_ASM("asm/levels", func_L00_003D1D28);

INCLUDE_ASM("asm/levels", func_L00_003D2A00);

INCLUDE_ASM("asm/levels", func_L00_003D2C00);

INCLUDE_ASM("asm/levels", func_L00_003D2ED0);

INCLUDE_ASM("asm/levels", func_L00_003D3388);
