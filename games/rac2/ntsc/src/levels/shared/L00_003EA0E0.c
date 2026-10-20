#include "include_asm.h"

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

extern int D_L00_001B0798[] __attribute__((sda));

extern void func_002A3F10(char *p, int one, int *w, char *out);

extern void func_00282D30_3(char *a, char *out, float f);

extern void func_00282CC0_4(char *a, char *b, char *c);

extern void func_00283098_2(char *a, char *b, char *c);

extern void func_002FDBD8_4(float scale, Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context,
                                    float *vector, int mode);

extern int func_002ACBC8(void *, int);

extern int func_L00_002E3B38(int);

extern void func_0026FEB8(void *, int, int);

extern unsigned char D_00189EA0[];

extern unsigned char D_0019B2E8[];

extern int D_001A79F0 __attribute__((sda));

extern int D_001A7A10 __attribute__((sda));

extern int D_001A7A14 __attribute__((sda));

extern int D_L00_001C5480[][3];

extern int D_L00_001C5540[][3];

extern char D_L00_001B0700[];

/* Measured partial layouts; original type and game roles are unknown. */

/* scales a float field by 1/1024, passes a caller word and two 16/48-byte scratch buffers to a four-argument helper, then forwards the second buffer with the scaled float to one helper and two sub-objects of the same parameter to three more helpers */
void func_L00_003EA0E0(char *p, int v, char *a, char *b)
{
    char out0[48];
    char out48[16];
    int w;
    float f;

    f = *(float *)(p + 44) * 0.0009765625f;
    w = v;
    func_002A3F10(p, 1, &w, out0);
    func_00282D30_3(a, out48, f);
    func_00282CC0_4(a, a, b);
    func_00283098_2(a, a, p + 192);
    func_00282CC0_4(a, a, p + 16);
}

INCLUDE_ASM("asm/levels", func_L00_003EA188);

INCLUDE_ASM("asm/levels", func_L00_003EA408);

INCLUDE_ASM("asm/levels", func_L00_003EA540);

/* shared native body of family 5a97ab89858c */
void func_L00_003EA860(char *p)
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

INCLUDE_ASM("asm/levels", func_L00_003EA8C8);

INCLUDE_ASM("asm/levels", func_L00_003EAA58);

/* When the context flag at offset 0x24 is nonzero, pass scale 1.0, the context, its vector at offset 0x10 and mode zero to the bound helper; original game role is unknown. */
void func_L00_003EAC88(Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context = owner->context_68;
    if (context->active_24 != 0) {
        func_002FDBD8_4(1.0f, context, context->vector_10, 0);
    }
}

void func_L00_003EAEF8(char *object) {
    int *state = *(int **)(object + 0x68);
    int result, index, shift;
    unsigned char *entry;

    if (!func_002ACBC8(D_00189EA0, state[1]))
        return;
    D_00189EA0[0x2246] = *(unsigned char *)(object + 0xB0);
    if (state[2])
        return;
    if ((D_0019B2E8[*(unsigned char *)(object + 0xB0) + D_001A79F0 * 16] >>
         (*(unsigned char *)(object + 0xB1) * 2)) & 3)
        return;
    state[2] = 1;
    result = func_L00_002E3B38(*(unsigned char *)(object + 0xB0));
    index = (*(unsigned char *)(object + 0xB0) & 15) + D_001A79F0 * 16;
    shift = (*(unsigned char *)(object + 0xB1) & 3) * 2;
    entry = D_0019B2E8 + index;
    if (!((*entry >> shift) & 3)) {
        *entry = (*entry & ~(3 << shift)) | (result << shift);
        D_001A7A10 += D_L00_001C5480[*(unsigned char *)(object + 0xB0) & 15]
                                   [*(unsigned char *)(object + 0xB1) & 3];
        D_001A7A14 += D_L00_001C5540[*(unsigned char *)(object + 0xB0) & 15]
                                   [*(unsigned char *)(object + 0xB1) & 3];
    }
    func_0026FEB8(D_L00_001B0700,
        D_L00_001C5480[*(unsigned char *)(object + 0xB0)][*(unsigned char *)(object + 0xB1)],
        D_L00_001C5540[*(unsigned char *)(object + 0xB0)][*(unsigned char *)(object + 0xB1)]);
}

INCLUDE_ASM("asm/levels", func_L00_003EB0C0);

INCLUDE_ASM("asm/levels", func_L00_003EB400);

INCLUDE_ASM("asm/levels", func_L00_003EB610);

INCLUDE_ASM("asm/levels", func_L00_003EB7B8);

INCLUDE_ASM("asm/levels", func_L00_003EB7C0);

INCLUDE_ASM("asm/levels", func_L00_003EB8E8);

INCLUDE_ASM("asm/levels", func_L00_003EBC68);

INCLUDE_ASM("asm/levels", func_L00_003EC078);

INCLUDE_ASM("asm/levels", func_L00_003EC140);

void func_L00_003ECB00(void)
{
    int i;

    for (i = 3; i >= 0; i--)
        D_L00_001B0798[i] = 0;
}

INCLUDE_ASM("asm/levels", func_L00_003ECB38);

INCLUDE_ASM("asm/levels", func_L00_003ECC88);

INCLUDE_ASM("asm/levels", func_L00_003ECC90);

INCLUDE_ASM("asm/levels", func_L00_003ECD80);

INCLUDE_ASM("asm/levels", func_L00_003ED198);

INCLUDE_ASM("asm/levels", func_L00_003ED850);

INCLUDE_ASM("asm/levels", func_L00_003EDB98);

INCLUDE_ASM("asm/levels", func_L00_003EDEE0);
