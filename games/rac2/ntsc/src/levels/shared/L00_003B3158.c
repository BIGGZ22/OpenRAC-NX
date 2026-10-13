#include "include_asm.h"
#include "common.h"

typedef struct OBJ {
    char pad0[32];
    unsigned char f20;
    char pad1[0x64 - 33];
    void (*f64)(struct OBJ *);
    char pad2[0xAA - 0x68];
    short fAA;
} OBJ;

extern OBJ *D_L06_001B2820_1 __attribute__((sda));

extern OBJ *D_L06_001B2824_1 __attribute__((sda));

extern OBJ *D_L06_001AC000 __attribute__((sda));

typedef struct NODE {
    char pad0[0x8];
    int f08;
    int f0C;
    char pad1[0x24 - 0x10];
    int f24;
    int f28;
    int f2C;
    unsigned char f30;
    char pad2[0x40 - 0x31];
    int f40;
    char pad3[0x68 - 0x44];
    void *f68;
    char pad4[0xB8 - 0x6C];
    int fB8;
    unsigned char fBC;
} NODE;

typedef struct V2 {
    float x;
    float y;
} V2;

extern V2 D_L00_001AFBC8;

extern void func_37CB80(void *, void *, float, void *);

extern float func_2EE048(float);

extern float ref_L00_003B7190_00 __attribute__((sda));

extern float ref_L00_003B7190_01 __attribute__((sda));

extern NODE *func_3100D8();

extern void func_31A658();

INCLUDE_ASM("asm/levels", func_L00_003B3158);

INCLUDE_ASM("asm/levels", func_L00_003B3450);

INCLUDE_ASM("asm/levels", func_L00_003B39C8);

INCLUDE_ASM("asm/levels", func_L00_003B39D0);

INCLUDE_ASM("asm/levels", func_L00_003B3AF8);

INCLUDE_ASM("asm/levels", func_L00_003B3C40);

INCLUDE_ASM("asm/levels", func_L00_003B3E78);

INCLUDE_ASM("asm/levels", func_L00_003B4018);

INCLUDE_ASM("asm/levels", func_L00_003B4198);

INCLUDE_ASM("asm/levels", func_L00_003B4560);

INCLUDE_ASM("asm/levels", func_L00_003B4E88);

INCLUDE_ASM("asm/levels", func_L00_003B4EF8);

INCLUDE_ASM("asm/levels", func_L00_003B5210);

INCLUDE_ASM("asm/levels", func_L00_003B52C8);

INCLUDE_ASM("asm/levels", func_L00_003B58B8);

INCLUDE_ASM("asm/levels", func_L00_003B5BF8);

INCLUDE_ASM("asm/levels", func_L00_003B60E8);

INCLUDE_ASM("asm/levels", func_L00_003B6158);

INCLUDE_ASM("asm/levels", func_L00_003B63D8);

INCLUDE_ASM("asm/levels", func_L00_003B6858);

INCLUDE_ASM("asm/levels", func_L00_003B69D8);

INCLUDE_ASM("asm/levels", func_L00_003B6A90);

INCLUDE_ASM("asm/levels", func_L00_003B6B70);

INCLUDE_ASM("asm/levels", func_L00_003B6F88);

void func_L00_003B7190(float *a0, float *a1)
{
    if (func_2EE048(*a0) > ref_L00_003B7190_00) {
        if (*a0 < -ref_L00_003B7190_00)
            *a0 = ref_L00_003B7190_00;
        else if (*a0 > ref_L00_003B7190_00)
            *a0 = -ref_L00_003B7190_00;
    }

    if (func_2EE048(*a1) > ref_L00_003B7190_01) {
        if (*a1 < -ref_L00_003B7190_01)
            *a1 = ref_L00_003B7190_01;
        else if (*a1 > ref_L00_003B7190_01)
            *a1 = -ref_L00_003B7190_01;
    }
}

INCLUDE_ASM("asm/levels", func_L00_003B72B0);

void *func_L00_003B7410(char *a0, void *a1, void *a2, int a3)
{
    NODE *o = func_3100D8(0x88D);

    if (o != 0) {
        char *d = (char *)o + 0x10;
        void *s = o->f68;

        o->fB8 = (int)a0;
        o->f30 = 0xFF;
        o->fBC = a3;
        qcopy(d, a1);
        qcopy(s, a2);
        func_31A658(o, 2, -1);
    }
    return o;
}

INCLUDE_ASM("asm/levels", func_L00_003B74A8);

/* shared native body of family 32969de2c02196ef */
void func_L00_003B75C8(void)
{
    OBJ *p;
    short key = D_L06_001AC000->fAA;

    for (p = D_L06_001B2820_1; p < D_L06_001B2824_1; p = (OBJ *)((char *)p + 256)) {
        if (p == 0)
            continue;
        if (p->fAA != key)
            continue;
        if (p->f20 == 254)
            continue;
        if (p->f20 == 253)
            continue;
        if (p == D_L06_001AC000)
            continue;
        p->f64(p);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003B7680);

INCLUDE_ASM("asm/levels", func_L00_003B77A8);

INCLUDE_ASM("asm/levels", func_L00_003B79A0);

INCLUDE_ASM("asm/levels", func_L00_003B7D48);

void func_L00_003B8158(NODE *a0)
{
    NODE *o = a0->f68;

    if (o->f24 == 0)
        return;
    if (o->f2C == 0) {
        func_37CB80(o, (char *)o + 0x10, 1.0f, 0);
    } else {
        V2 v = D_L00_001AFBC8;

        func_37CB80(o, (char *)o + 0x10, 1.0f, &v);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003B81D0);

INCLUDE_ASM("asm/levels", func_L00_003B8420);

INCLUDE_ASM("asm/levels", func_L00_003B84F0);

INCLUDE_ASM("asm/levels", func_L00_003B8E58);

INCLUDE_ASM("asm/levels", func_L00_003B9170);

INCLUDE_ASM("asm/levels", func_L00_003B9310);
