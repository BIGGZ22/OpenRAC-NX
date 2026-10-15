#include "include_asm.h"

extern char *Ffc961fca_FUN_002AD0B0(char *a1);

extern void Ffc961fca_FUN_00283410(char *dst, char *src);

extern void Ffc961fca_FUN_00282CC0(char *dst, char *src, char *tail);

extern void Ffc961fca_FUN_00282CF0(char *dst, char *src, char *tail);

extern void Ffc961fca_FUN_00283698(char *dst, char *src);

extern void Ffc961fca_FUN_00283098(char *dst, char *src, char *tail);

extern void Ffc961fca_FUN_00283738(char *dst, char *src, char *tail);

typedef struct {
    char pad0[0x20];
    unsigned char state;
} Level16VendorMoby;

extern void Fc936841d_FUN_00282CF0(char *local, char *first, char *second);

extern void Fc936841d_FUN_00282CC0(char *target, char *first, char *local);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

INCLUDE_ASM("asm/game", func_0030F7B0);

INCLUDE_ASM("asm/game", func_00310960);

INCLUDE_ASM("asm/game", func_00310A40);

INCLUDE_ASM("asm/game", func_00310C88);

INCLUDE_ASM("asm/game", func_00310D70);

void func_00310D78(char *o0, char *o1)
{
    char b0[64];
    char b1[64];
    char b2[64];
    char *r = Ffc961fca_FUN_002AD0B0(o1);

    if (r == 0)
        return;
    Ffc961fca_FUN_00283410(b0, r);
    Ffc961fca_FUN_00282CC0(o0 + 16, o0 + 16, r + 16);
    Ffc961fca_FUN_00282CF0(o0 + 16, o0 + 16, o1 + 16);
    if ((*(int *)(r + 60) & 2) != 0) {
        Ffc961fca_FUN_00283410(b2, r + 32);
        Ffc961fca_FUN_00283698(b1, b2);
        Ffc961fca_FUN_00283098(o0 + 16, o0 + 16, b1);
        Ffc961fca_FUN_00283098(o0 + 16, o0 + 16, o1 + 192);
    } else {
        Ffc961fca_FUN_00283098(o0 + 16, o0 + 16, b0);
    }
    Ffc961fca_FUN_00282CC0(o0 + 16, o0 + 16, o1 + 16);
    Ffc961fca_FUN_00283738(o0 + 192, b0, o0 + 192);
}

INCLUDE_ASM("asm/game", func_00310E98);

INCLUDE_ASM("asm/game", func_003116E8);

INCLUDE_ASM("asm/game", func_00311F70);

INCLUDE_ASM("asm/game", func_003120C8);

INCLUDE_ASM("asm/game", func_003125F8);

INCLUDE_ASM("asm/game", func_00312600);

int func_00312B58(Level16VendorMoby *moby) {
    return moby->state == 6;
}

INCLUDE_ASM("asm/game", func_00312B68);

INCLUDE_ASM("asm/game", func_00312B70);

INCLUDE_ASM("asm/game", func_00312D48);

INCLUDE_ASM("asm/game", func_00312D58);

INCLUDE_ASM("asm/game", func_00312E00);

int func_00312E10(unsigned char *p) { return p[0x20] == 1; }

INCLUDE_ASM("asm/game", func_00312E20);

INCLUDE_ASM("asm/game", func_00312EA8);

INCLUDE_ASM("asm/game", func_00312EB0);

INCLUDE_ASM("asm/game", func_003130F0);

INCLUDE_ASM("asm/game", func_00313210);

extern char D_L00_0023A670[][16];
extern void func_002ACEC0(void *, void *, void *, float);
extern void func_00282F20(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);

typedef struct { char pad[0x30]; char *entries[1]; } SolEmitterData;

void func_00313548(unsigned char *object, float *out, int index)
{
    char local[16];
    char *data = *(char **)(object + 0x68);
    char *entry;
    func_002ACEC0(local, D_L00_0023A670[index], data + 0x20, *(float *)(data + 0x2C));
    entry = ((SolEmitterData *)data)->entries[index];
    if (entry && object[0x20] == 2)
        func_00282F20(local, local, *(float *)(entry + 0x28));
    func_00282CC0(out, data + 0x10, local);
    out[2] += *(float *)data;
}

INCLUDE_ASM("asm/game", func_00313600);

INCLUDE_ASM("asm/game", func_00313678);

INCLUDE_ASM("asm/game", func_003136D0);

INCLUDE_ASM("asm/game", func_003146C0);

INCLUDE_ASM("asm/game", func_00314CF8);

INCLUDE_ASM("asm/game", func_00314E28);

INCLUDE_ASM("asm/game", func_00314FD0);

INCLUDE_ASM("asm/game", func_00315890);

INCLUDE_ASM("asm/game", func_003158A8);

INCLUDE_ASM("asm/game", func_003159D0);

INCLUDE_ASM("asm/game", func_00315C50);

INCLUDE_ASM("asm/game", func_00315C78);

INCLUDE_ASM("asm/game", func_00315E20);

INCLUDE_ASM("asm/game", func_00316248);

INCLUDE_ASM("asm/game", func_00316400);

INCLUDE_ASM("asm/game", func_003166C8);

INCLUDE_ASM("asm/game", func_003169D0);

void func_00316B40(char *p)
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

INCLUDE_ASM("asm/game", func_00316BC8);

void func_00316DA0(Vec16_Fc936841d *object, int index)
{
    char local[16];

    Fc936841d_FUN_00282CF0(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    Fc936841d_FUN_00282CC0((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/game", func_00316E08);

INCLUDE_ASM("asm/game", func_003172D0);
