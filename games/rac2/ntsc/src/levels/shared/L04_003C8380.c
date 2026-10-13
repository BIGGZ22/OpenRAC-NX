#include "include_asm.h"

extern int func_002A9640(unsigned char *a0, int a1, int a2);

extern void func_002AB800(unsigned char *a0, unsigned char *a1);

extern void func_002AB8C0(unsigned char *a0, unsigned char *a1);

extern void *func_0029EFD0(void *a0);

extern void func_002A0F48(void *a0);

extern int func_002A8508(void *a0, int a1);

extern int func_002A77E0(int a0);

extern void func_L04_003F9078(unsigned char *a0);

#ifndef RAC2_T_Q16_B0E5D8F83
#define RAC2_T_Q16_B0E5D8F83
typedef int Q16_B0e5d8f83 __attribute__((mode(TI)));
#endif

INCLUDE_ASM("asm/levels", func_L04_003C8380);

INCLUDE_ASM("asm/levels", func_L04_003E5830);

void *func_L04_003E5E40(unsigned char *a0, void *a1, float f12) {
    unsigned char *obj = func_0029EFD0(a1);

    if (obj != 0) {
        obj[0x30] = 0xFF;
        obj[0x31] = 1;
        *(unsigned short *)(obj + 0x32) = 0xFF;
        obj[0x20] = 0;
        *(float *)(obj + 0x2C) = *(float *)(*(unsigned char **)(obj + 0x24) + 0x24) * f12;
        *(Q16_B0e5d8f83 *)(obj + 0x10) = *(Q16_B0e5d8f83 *)(a0 + 0x10);
        *(Q16_B0e5d8f83 *)(obj + 0xF0) = *(Q16_B0e5d8f83 *)(a0 + 0xF0);
        obj[0x62] = 1;
        func_002A0F48(obj);
    }
    return obj;
}

INCLUDE_ASM("asm/levels", func_L04_003EDD40);

void func_L04_003EDF80(unsigned char *a0, unsigned char *a1) {
    if (func_002A9640(a0, 0x10000, 0) != 0) {
        a0[0x20] = 2;
        a1[0x57] = 0x78;
        func_002AB800(a0, a1 + 0x50);
    }
    a0[0xA8] = 0xFF;
    func_002AB8C0(a0, a1 + 0x50);
}

INCLUDE_ASM("asm/levels", func_L04_003EDFF0);

INCLUDE_ASM("asm/levels", func_L04_003EE090);

INCLUDE_ASM("asm/levels", func_L04_003EE7C0);

INCLUDE_ASM("asm/levels", func_L04_003F1388);

void func_L04_003F5F30(unsigned char *a0) {
    unsigned char *obj = *(unsigned char **)(a0 + 0x68);
    float *p;
    unsigned int *q;
    unsigned int v;
    int i;
    int j;

    p = (float *)(obj + 0x14);
    for (i = 3; i >= 0; i--) {
        *p++ = (float)func_002A77E0(0xFF);
    }
    *(float *)(obj + 0x24) = -1.0f;
    *(float *)(obj + 0x28) = -2.25f;
    *(float *)(obj + 0x2C) = 1.25f;
    *(float *)(obj + 0x30) = 2.5f;
    j = 3;
    q = (unsigned int *)(obj + 0x40);
    v = 0xFF;
    for (; j >= 0; j--) {
        *q = v;
        q--;
        v -= 0x40;
    }
    *(float *)(obj + 0x44) = 2.0f;
    *(float *)(obj + 0x48) = 2.5f;
    *(float *)(obj + 0x4C) = 2.5f;
    *(float *)(obj + 0x50) = 2.0f;
}

INCLUDE_ASM("asm/levels", func_L04_003F6008);

INCLUDE_ASM("asm/levels", func_L04_003F6200);

void func_L04_003F8FA0(unsigned char *a0) {
    unsigned char *obj = *(unsigned char **)(a0 + 0x68);
    float *p;
    unsigned int *q;
    unsigned int v;
    int i;
    int j;

    p = (float *)(obj + 0x10);
    for (i = 3; i >= 0; i--) {
        *p++ = (float)func_002A77E0(0xFF);
    }
    *(float *)(obj + 0x20) = -1.0f;
    *(float *)(obj + 0x24) = -2.25f;
    *(float *)(obj + 0x28) = 1.25f;
    *(float *)(obj + 0x2C) = 2.5f;
    j = 3;
    q = (unsigned int *)(obj + 0x3C);
    v = 0xFF;
    for (; j >= 0; j--) {
        *q = v;
        q--;
        v -= 0x40;
    }
    *(float *)(obj + 0x40) = 2.0f;
    *(float *)(obj + 0x44) = 2.5f;
    *(float *)(obj + 0x48) = 2.5f;
    *(float *)(obj + 0x4C) = 2.0f;
}

INCLUDE_ASM("asm/levels", func_L04_003F9078);

INCLUDE_ASM("asm/levels", func_L04_003F9320);

INCLUDE_ASM("asm/levels", func_L04_003F9718);

INCLUDE_ASM("asm/levels", func_L04_003F9C60);

INCLUDE_ASM("asm/levels", func_L04_003F9ED0);

INCLUDE_ASM("asm/levels", func_L04_003FA350);

INCLUDE_ASM("asm/levels", func_L04_003FA5A8);

INCLUDE_ASM("asm/levels", func_L04_003FA7E0);

INCLUDE_ASM("asm/levels", func_L04_003FB990);

INCLUDE_ASM("asm/levels", func_L04_003FBD80);

INCLUDE_ASM("asm/levels", func_L04_003FBEF8);

INCLUDE_ASM("asm/levels", func_L04_003FC678);

void *func_L04_00400A18(unsigned char *a0, void *a1, float f12) {
    unsigned char *obj = func_0029EFD0(a1);

    if (obj != 0) {
        obj[0x30] = 0xFF;
        *(unsigned short *)(obj + 0x32) = 0xFF;
        obj[0x31] = 1;
        obj[0x20] = 0;
        *(float *)(obj + 0x2C) = *(float *)(*(unsigned char **)(obj + 0x24) + 0x24) * f12;
        *(Q16_B0e5d8f83 *)(obj + 0x10) = *(Q16_B0e5d8f83 *)(a0 + 0x10);
        *(Q16_B0e5d8f83 *)(obj + 0xF0) = *(Q16_B0e5d8f83 *)(a0 + 0xF0);
        func_002A0F48(obj);
    }
    return obj;
}

INCLUDE_ASM("asm/levels", func_L04_00400AB0);

INCLUDE_ASM("asm/levels", func_L04_00400D48);

INCLUDE_ASM("asm/levels", func_L04_00400EE0);

INCLUDE_ASM("asm/levels", func_L04_004027B0);

INCLUDE_ASM("asm/levels", func_L04_004029D8);

INCLUDE_ASM("asm/levels", func_L04_00402C88);
