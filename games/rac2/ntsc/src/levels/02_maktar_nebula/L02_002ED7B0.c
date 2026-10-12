#include "include_asm.h"
#include "common.h"

extern void func_002A7750(void *, int, int);

extern void func_002A7470(void *, int, int, int);

extern int func_00282978(short *);

extern int func_L02_0032E998(char *, int, float *, char *, short, char *, char *, float, float);

extern int func_002B50E0(char *, int, float);

extern void func_002AF1D0(char *, char *, int, int);

extern void func_002B0990(char *, char *, float, int, int);

extern void func_002A8C28(char *, char *, int, float, float, float, int, int, int);

void func_L02_003CADA8(char *arg0, char *arg1) {
    char *p = *(char **)(arg0 + 0x68);
    func_002A7750(arg0, 7, -1);
    qcopy(p + 0xF0, arg1);
}

void func_L02_003D01F0(char *arg0, char *arg1) {
    char *self = arg0;
    char *p = *(char **)(self + 0x68);
    func_002A7750(self, 4, -1);
    qcopy(p + 0xB0, arg1);
}

int func_L02_003D1880(char *arg0) {
    int v = 0;
    if (*(int *)(*(char **)(arg0 + 0x68) + 0x2E4) != 0) {
        unsigned char c = *(unsigned char *)(arg0 + 0x20);
        if (c == 0x13 || c == 4 || *(unsigned char *)(arg0 + 0x95) == 4 ||
            c == 0x10 || c == 0x11 || c == 0x12)
            v = 1;
    }
    return v;
}

int func_L02_003CADF0(char *self, int arg1) {
    float out;
    char *p = *(char **)(self + 0x68);
    int r = func_L02_0032E998(self + 0x10, arg1, &out, p + 0x290, *(short *)(p + 0x3B6),
                              p + 0x3B8, p + 0x3BC, 0.0f, *(float *)(p + 0x50));
    func_002B50E0(self, arg1, out);
    return r;
}

void func_L02_003D14F8(char *self) {
    char *p = *(char **)(self + 0x68);
    if (*(int *)(p + 0x2E4) != 0) {
        func_002A7750(self, 4, 2);
        if (*(unsigned char *)(self + 0x43) != 0)
            func_002A7470(self, 0, 0, 0xA);
    } else {
        func_002A7750(self, 5, 2);
        if (*(unsigned char *)(self + 0x43) != 1)
            func_002A7470(self, 1, 0, 0xA);
    }
}

void func_L02_003D1430(char *self) {
    char *p = *(char **)(self + 0x68);
    func_00282978((short *)(p + 0x2F4));
    if (*(unsigned short *)(self + 0x96) < 0xFFFF)
        (*(unsigned short *)(self + 0x96))++;
    if (*(int *)(p + 0x2E4) != 0) {
        func_002AF1D0(self, *(char **)(p + 0x2E4), 2, 0);
        *(unsigned short *)(*(char **)(p + 0x2E4) + 0x34) &= 0xFFFD;
    }
    func_002B0990(self, p + 0x210, 3.5f, 4, 1);
    func_002A8C28(self, self + 0x10, 0x4000000, 1.5f, 1.0f, 1.0f, 0, 1, 0);
}

extern void *func_0029EFD0(int);

extern void func_002A0F48(char *);

void func_L02_003D1E40(char *self) {
    char *p = *(char **)(self + 0x68);
    *(char **)(p + 0x2E4) = func_0029EFD0(0x9D8);
    if (*(char **)(p + 0x2E4) != 0) {
        *(unsigned char *)(*(char **)(p + 0x2E4) + 0x30) = *(unsigned char *)(self + 0x30);
        *(unsigned short *)(*(char **)(p + 0x2E4) + 0x32) = *(unsigned short *)(self + 0x32);
        *(unsigned char *)(*(char **)(p + 0x2E4) + 0x31) = 1;
        *(unsigned long long *)(*(char **)(p + 0x2E4) + 0x38) =
            *(unsigned long long *)(self + 0x38);
        func_002A0F48(*(char **)(p + 0x2E4));
        *(int *)(*(char **)(p + 0x2E4) + 0x98) = 0;
        *(int *)(*(char **)(*(char **)(p + 0x2E4) + 0x68) + 0x3C) = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L02_002ED7B0);

INCLUDE_ASM("asm/levels", func_L02_002F2B88);

INCLUDE_ASM("asm/levels", func_L02_00300738);

INCLUDE_ASM("asm/levels", func_L02_003007D8);

INCLUDE_ASM("asm/levels", func_L02_00300CE8);

INCLUDE_ASM("asm/levels", func_L02_0033C760);

INCLUDE_ASM("asm/levels", func_L02_0033C8B8);

INCLUDE_ASM("asm/levels", func_L02_0035BD60);

INCLUDE_ASM("asm/levels", func_L02_00363D78);

ASM_FUNC("asm/levels", func_L02_00364A18);

ASM_FUNC("asm/levels", func_L02_00364D3C);

INCLUDE_ASM("asm/levels", func_L02_00397D10);

INCLUDE_ASM("asm/levels", func_L02_00398128);

INCLUDE_ASM("asm/levels", func_L02_0039D708);

INCLUDE_ASM("asm/levels", func_L02_003CA4A8);

INCLUDE_ASM("asm/levels", func_L02_003CAE68);

INCLUDE_ASM("asm/levels", func_L02_003CB090);

INCLUDE_ASM("asm/levels", func_L02_003CB0F0);

INCLUDE_ASM("asm/levels", func_L02_003CD5B0);

INCLUDE_ASM("asm/levels", func_L02_003CD668);

INCLUDE_ASM("asm/levels", func_L02_003CFAC8);

INCLUDE_ASM("asm/levels", func_L02_003D0238);

INCLUDE_ASM("asm/levels", func_L02_003D02F0);

INCLUDE_ASM("asm/levels", func_L02_003D0408);

INCLUDE_ASM("asm/levels", func_L02_003D10F0);

INCLUDE_ASM("asm/levels", func_L02_003D1698);

INCLUDE_ASM("asm/levels", func_L02_003D18D8);

INCLUDE_ASM("asm/levels", func_L02_003D1998);

INCLUDE_ASM("asm/levels", func_L02_003D1C38);

INCLUDE_ASM("asm/levels", func_L02_003D1ED0);

INCLUDE_ASM("asm/levels", func_L02_003D2248);

INCLUDE_ASM("asm/levels", func_L02_003D2390);

INCLUDE_ASM("asm/levels", func_L02_003D2488);

INCLUDE_ASM("asm/levels", func_L02_003D25C8);
