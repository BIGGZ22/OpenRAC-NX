#include "include_asm.h"

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

INCLUDE_ASM("asm/levels", func_L00_003B7190);

INCLUDE_ASM("asm/levels", func_L00_003B7260);

INCLUDE_ASM("asm/levels", func_L00_003B72B0);

INCLUDE_ASM("asm/levels", func_L00_003B7410);

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

INCLUDE_ASM("asm/levels", func_L00_003B7908);

INCLUDE_ASM("asm/levels", func_L00_003B79A0);

INCLUDE_ASM("asm/levels", func_L00_003B7D48);

INCLUDE_ASM("asm/levels", func_L00_003B8138);

INCLUDE_ASM("asm/levels", func_L00_003B8158);

INCLUDE_ASM("asm/levels", func_L00_003B81D0);

INCLUDE_ASM("asm/levels", func_L00_003B8420);

INCLUDE_ASM("asm/levels", func_L00_003B84F0);

INCLUDE_ASM("asm/levels", func_L00_003B8E58);

INCLUDE_ASM("asm/levels", func_L00_003B9170);

INCLUDE_ASM("asm/levels", func_L00_003B9290);

INCLUDE_ASM("asm/levels", func_L00_003B92D0);

INCLUDE_ASM("asm/levels", func_L00_003B9310);
