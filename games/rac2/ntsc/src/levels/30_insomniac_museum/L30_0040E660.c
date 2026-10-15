#include "include_asm.h"

extern void func_42C028(int);
extern void func_316DB8(char *);
extern int D_0018C0E8;

void func_L30_00418670(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    func_42C028(3);
    D_0018C0E8 = *(int *)(st + 0x5C);
    func_316DB8(*(char **)(st + 0x50));
}

INCLUDE_ASM("asm/levels", func_L30_0040E660);

INCLUDE_ASM("asm/levels", func_L30_004130E8);

INCLUDE_ASM("asm/levels", func_L30_00413490);

INCLUDE_ASM("asm/levels", func_L30_00413528);

INCLUDE_ASM("asm/levels", func_L30_004135A0);

INCLUDE_ASM("asm/levels", func_L30_004148F0);

INCLUDE_ASM("asm/levels", func_L30_00415090);

INCLUDE_ASM("asm/levels", func_L30_004153D8);

INCLUDE_ASM("asm/levels", func_L30_004158B0);

INCLUDE_ASM("asm/levels", func_L30_00416A88);

INCLUDE_ASM("asm/levels", func_L30_00416D88);

INCLUDE_ASM("asm/levels", func_L30_00417988);

typedef unsigned short u16;

extern char *func_316A50(int);
extern void func_32BFF0(void *, void *);
extern void func_2F56B0(void *, void *);

void func_L30_00418570(char *a0) {
    char *st = *(char **)(a0 + 0x68);
    char *o;

    o = func_316A50(0xB28);
    if (o == 0) return;
    func_32BFF0(o, a0);
    func_2F56B0(o + 0xC0, o + 0xF0);
    *(u16 *)(o + 0x34) |= 0x100;
    *(char **)(st + 0x50) = o;
}

INCLUDE_ASM("asm/levels", func_L30_004185E0);

INCLUDE_ASM("asm/levels", func_L30_004186B0);

INCLUDE_ASM("asm/levels", func_L30_004191E0);

extern int func_2F4B08(char *);
extern void func_2EDD78(void *, char *);
extern void func_L00_00415AA0(char *);

void func_L30_0041A848(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    if (func_2F4B08(st) == 0) {
        func_2F4B08(st + 4);
        func_2EDD78((void *)func_L00_00415AA0, a0);
    } else {
        func_316DB8(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L30_0041B848);
