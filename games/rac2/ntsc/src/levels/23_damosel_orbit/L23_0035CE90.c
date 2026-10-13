#include "include_asm.h"

extern void func_3242D0(int, int, int);

extern void func_L23_003C1988(char *arg0);

extern int D_L23_001B36B0 __attribute__((sda));

INCLUDE_ASM("asm/levels", func_L23_0035CE90);

INCLUDE_ASM("asm/levels", func_L23_00394218);

INCLUDE_ASM("asm/levels", func_L23_00394630);

INCLUDE_ASM("asm/levels", func_L23_00399C10);

INCLUDE_ASM("asm/levels", func_L23_003AAC30);

INCLUDE_ASM("asm/levels", func_L23_003BDA20);

INCLUDE_ASM("asm/levels", func_L23_003BEA00);

INCLUDE_ASM("asm/levels", func_L23_003BF9B0);

extern void func_2F7DD0(char *);

extern void func_2F7E00(char *);

void func_L23_003BFE38(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    func_2F7DD0(state + 0x250);
    func_2F7E00(state + 0x254);
    if (*(unsigned short *)(arg0 + 0x96) <= 0xFFFE)
        *(unsigned short *)(arg0 + 0x96) += 1;
}

INCLUDE_ASM("asm/levels", func_L23_003BFE90);

extern void func_323FF0(char *, int, int, int);

void func_L23_003BFFC0(char *arg0, int arg1, int arg2, int arg3, int arg4, float arg5) {
    char *state = *(char **)(arg0 + 0x68);

    if (arg4 & 1)
        func_323FF0(arg0, arg1, 0, 4);
    if (arg4 & 2)
        func_323FF0(*(char **)(state + 0x2A0), arg1, 0, 4);
}

extern void func_367150(int);

void func_L23_003C0028(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int none = -1;

    func_L23_003C1988(arg0);
    *(short *)(state + 0x178) = 0;
    *(unsigned short *)(*(char **)(state + 0x2A0) + 0x34) |= 0x41;
    *(unsigned short *)(*(char **)(state + 0x2A0) + 0x34) &= 0xEFFF;
    *(int *)(*(char **)(state + 0x2A0) + 0x98) = 0;
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_3242D0(arg0, 0xD, 4);
    *(int *)(state + 0x25C) = 0;
    if (*(int *)(state + 0x3B4) != none) {
        func_367150(*(int *)(state + 0x3B4));
        *(int *)(state + 0x3B4) = none;
    }
}

INCLUDE_ASM("asm/levels", func_L23_003C00D0);

INCLUDE_ASM("asm/levels", func_L23_003C01F8);

INCLUDE_ASM("asm/levels", func_L23_003C02C8);

INCLUDE_ASM("asm/levels", func_L23_003C0A60);

INCLUDE_ASM("asm/levels", func_L23_003C0C78);

extern int func_419CA0(int);

int func_L23_003C0DF8(int arg0, unsigned long long v, int *arg2, int arg3) {
    int *out = arg2;
    int i = 0;
    int n = 0;

    while (i < 0x40) {
        if (v & 1) {
            if (arg3 == 0 || func_419CA0(i) != 0) {
                *out = i;
                n++;
                out++;
            }
        }
        i++;
        v >>= 1;
    }
    return n;
}

INCLUDE_ASM("asm/levels", func_L23_003C0E98);

void func_L23_003C0F48(char *arg0, short arg1) {
    char *p = *(char **)(arg0 + 0x68);
    short n = *(short *)(p + 0x2BE);
    unsigned short old = *(unsigned short *)(p + 0x2B4);
    int off;
    int r;

    r = (n + 1) % 3;
    off = r * 2;
    *(short *)(p + 0x2BE) = r;
    *(short *)(p + off + 0x2B8) = old;
    *(short *)(p + 0x2B4) = arg1;
}

INCLUDE_ASM("asm/levels", func_L23_003C0F88);

INCLUDE_ASM("asm/levels", func_L23_003C1420);

INCLUDE_ASM("asm/levels", func_L23_003C16D8);

void func_L23_003C1960(char *arg0) {
    D_L23_001B36B0 = 0;
    func_L23_003C1988(arg0);
}

extern void func_31A108(int);

void func_L23_003C1988(char *arg0) {
    int *p = (int *)(*(int *)(arg0 + 0x68) + 0x328);
    int i;

    for (i = 1; i >= 0; i--) {
        int v = *p;

        if (v != 0) {
            func_31A108(v);
            *p = 0;
        }
        p++;
    }
}

INCLUDE_ASM("asm/levels", func_L23_003C19E0);

void func_L23_003C1AB0(int arg0) {
    func_3242D0(arg0, 9, 2);
    D_L23_001B36B0 = arg0;
}


INCLUDE_ASM("asm/levels", func_L23_003C1B58);

INCLUDE_ASM("asm/levels", func_L23_003C1E88);

INCLUDE_ASM("asm/levels", func_L23_003D4140);

INCLUDE_ASM("asm/levels", func_L23_00404BA8);

INCLUDE_ASM("asm/levels", func_L23_00415988);


void func_L23_00416090(char *arg0) {
    if (*(unsigned short *)(arg0 + 0x96) <= 0xFFFE)
        *(unsigned short *)(arg0 + 0x96) += 1;
}

INCLUDE_ASM("asm/levels", func_L23_004160B0);
