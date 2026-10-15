#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L11_004618C0);

INCLUDE_ASM("asm/levels", func_L11_0046D660);

INCLUDE_ASM("asm/levels", func_L11_0046F8F0);

INCLUDE_ASM("asm/levels", func_L11_004701A0);

INCLUDE_ASM("asm/levels", func_L11_004702F0);

void func_L11_00472DE8(unsigned char *a0) {
    unsigned short v;

    if (a0[0x20] != 0)
        return;
    v = *(unsigned short *)(a0 + 0x34);
    a0[0x20] = 1;
    *(unsigned short *)(a0 + 0x34) = v | 0x41;
}

INCLUDE_ASM("asm/levels", func_L11_00472E10);

INCLUDE_ASM("asm/levels", func_L11_00474920);

INCLUDE_ASM("asm/levels", func_L11_0047A310);

extern char *r1_D_L08_00167680;

/* rac1-decomp src/overlays/shared/vendor_002D3DF8.c: func_L08_00315068 */
void func_L11_004885A0(void) {
    r1_D_L08_00167680[0x88] = 1;
}

extern void func_803004A8(void *);
extern void func_80300D98(void *, void *);
extern void func_80301000(void *, void *);

void func_L11_004885B8(unsigned char *a0) {
    unsigned char buf[0x40];
    unsigned char *p;

    *(int *)(*(unsigned char **)(a0 + 0x70) + 0xC0) = 4;
    p = *(unsigned char **)(a0 + 0x70) + 0x30;
    *(float *)(p + 0x20) = 0.05f;
    *(float *)(p + 0x24) = 0.3f;
    *(float *)(p + 0x28) = 1.0f;
    *(unsigned short *)(p + 0x2C) = 1;
    func_803004A8(p);
    {
        unsigned char *q = *(unsigned char **)(a0 + 0x70);

        *(float *)(q + 0x10) = 0.3f;
        *(float *)(q + 0x14) = 0.3f;
        *(float *)(q + 0x18) = 1.0f;
        *(unsigned short *)(q + 0x1C) = 1;
        func_803004A8(q);
    }
    func_80300D98(buf, a0 + 0x40);
    func_80301000(a0, buf);
    *(unsigned short *)(a0 + 0x7E) = 0;
}

INCLUDE_ASM("asm/levels", func_L11_004A0CE0);

INCLUDE_ASM("asm/levels", func_L11_004A6B50);

INCLUDE_ASM("asm/levels", func_L11_004A8AB8);
