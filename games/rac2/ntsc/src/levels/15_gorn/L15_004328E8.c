#include "include_asm.h"

/* ---- shared declarations ---- */

extern int func_002ED688(unsigned char *owner);
extern void func_L05_0037AEC0(char *object, int flag);
extern void func_L05_00379C68(int arg0, int value);
extern void func_L00_002E6DB8(void *, void *);
extern void func_L00_002E6CF0(void *a0, void *a1);
extern void func_L00_002E6EF8(void *, int);
extern void func_L15_00438238(void);
extern void func_L15_00438260(void);
extern void func_L15_00438518(void);

INCLUDE_ASM("asm/levels", func_L15_004328E8);

INCLUDE_ASM("asm/levels", func_L15_00432A60);

INCLUDE_ASM("asm/levels", func_L15_00434100);

INCLUDE_ASM("asm/levels", func_L15_00434610);

INCLUDE_ASM("asm/levels", func_L15_00434D28);

INCLUDE_ASM("asm/levels", func_L15_00434F90);

INCLUDE_ASM("asm/levels", func_L15_004351E0);

INCLUDE_ASM("asm/levels", func_L15_004353E8);

INCLUDE_ASM("asm/levels", func_L15_00435838);

INCLUDE_ASM("asm/levels", func_L15_00435E80);

void func_L15_00436090(char *a0) {
    char *v1 = *(char **)(a0 + 0x68);

    *(int *)(v1 + 0x270) = 1;
    *(int *)(v1 + 0x274) = 0x1E0;
    *(char *)(a0 + 0x23) = 0x20;
    *(char *)(a0 + 0x20) = 2;
    *(unsigned short *)(a0 + 0x34) |= 8;
    *(float *)(v1 + 0x180) = 450.0f;
    *(int *)(a0 + 0x98) = 0;
}

INCLUDE_ASM("asm/levels", func_L15_004360D8);

INCLUDE_ASM("asm/levels", func_L15_00436298);

INCLUDE_ASM("asm/levels", func_L15_004363C8);

INCLUDE_ASM("asm/levels", func_L15_00436990);

void func_L15_00436BA8(char *a0) {
    char *s0 = a0;
    char *s1 = *(char **)(s0 + 0x68);
    char *s2 = (char *)func_002ED688((unsigned char *)a0);

    *(int *)(s2 + 0x148) = 100;
    func_L05_0037AEC0(s0, 1);
    *(float *)(s1 + 0x180) = 100.0f;
    *(unsigned char *)(s0 + 0x20) = 1;
    *(unsigned char *)(s0 + 0x30) = 0xFF;
    *(unsigned short *)(s0 + 0x32) = 0x1FF;
    *(unsigned short *)(s0 + 0x34) |= 0x100;
    *(float *)(s0 + 0x2C) = 0.20000000298023224f;
    *(float *)(s1 + 0x278) = 10.0f;
    *(int *)(s1 + 0x26C) = 0;
    *(int *)(s1 + 0x270) = 0;
    *(int *)(s1 + 0x274) = 0;
    *(int *)(s2 + 0x154) = (int)(s1 + 0x278);
    func_L05_00379C68((int)s0, 0xFFFF);
}

INCLUDE_ASM("asm/levels", func_L15_00436C68);

void func_L15_00437B18(void *a0) {
    func_L00_002E6DB8((void *)func_L15_00438238, a0);
    func_L00_002E6CF0((void *)func_L15_00438260, a0);
    func_L00_002E6EF8((void *)func_L15_00438518, a0);
}

INCLUDE_ASM("asm/levels", func_L15_00437B68);

INCLUDE_ASM("asm/levels", func_L15_00438260);

INCLUDE_ASM("asm/levels", func_L15_00438518);

INCLUDE_ASM("asm/levels", func_L15_00438B00);

INCLUDE_ASM("asm/levels", func_L15_004390A0);
