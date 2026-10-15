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

typedef int TI128 __attribute__((mode(TI)));

typedef struct {
    int x, y, z, w;
} V16;

extern void func_31E6A0(char *, int, int);

extern void func_31E3C0(char *, int, int, int);

extern void func_31E2E8(char *, int, int);

extern void func_315578(char *, int, void *);

extern void func_2F2A78(void *, void *);

extern void func_2EF388(char *, char *, char *, char *, int);

extern void func_L02_003D1ED0(char *, TI128 *);

extern void func_L02_003D2390(char *);

extern void func_3FC1B0(char *, int);

extern void func_315348(char *, char *);

extern void func_2F1CC8(char *, int, int);

extern void func_316220(char *);

extern void func_3152D0(char *, int, char *);

extern void func_31EA58(char *, float, float);

extern void func_32B068(char *, int, char *, int, float, float, float, float);

extern void func_3FC248(char *, int, int);

extern void func_40D7B8(char *, int, void *);

extern int func_L02_003D18D8(char *);

extern float func_2F2168(char *, char *, float);

extern float func_2F2648(float, float);

extern float func_2F30E8(float, float);

extern int func_31E770(int);

extern void func_332710(float, float, float, float);

extern void func_316150(char *, int, int, int);

extern void func_316180(char *, int *, int *, int *);

/* Stores the quadword the caller passes into the record at +0xF0. */
INCLUDE_ASM("asm/levels", func_L02_003CADA8);

/* Stores the quadword the caller passes into the record at +0xB0. */
INCLUDE_ASM("asm/levels", func_L02_003D01F0);

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

/* Resets the actor's aim record and hands its descriptor to the object system. */
INCLUDE_ASM("asm/levels", func_L02_003CD668);

INCLUDE_ASM("asm/levels", func_L02_003CFAC8);

INCLUDE_ASM("asm/levels", func_L02_003D0238);

INCLUDE_ASM("asm/levels", func_L02_003D02F0);

INCLUDE_ASM("asm/levels", func_L02_003D0408);

INCLUDE_ASM("asm/levels", func_L02_003D10F0);

/* Picks the idle variant the actor should play, from its distance and heading. */
INCLUDE_ASM("asm/levels", func_L02_003D1698);

INCLUDE_ASM("asm/levels", func_L02_003D18D8);

INCLUDE_ASM("asm/levels", func_L02_003D1998);

INCLUDE_ASM("asm/levels", func_L02_003D1C38);

INCLUDE_ASM("asm/levels", func_L02_003D1ED0);

/* Brings the actor into its armed state and re-registers its sub-object. */
void func_L02_003D2248(char *arg0, int arg1) {
    char *p = *(char **)(arg0 + 0x68);

    *(short *)(p + 0x2F6) = 0;
    *(float *)(p + 0x20) = (float)*(short *)(p + 0x24);
    func_332710(0.0055555556900799274f, 0.10333333909511566f, 0.0034906589426100254f,
                0.41887909173965454f);
    func_31E6A0(arg0, 3, 1);
    if (*(unsigned char *)(arg0 + 0x43) != 0)
        func_31E3C0(arg0, 0, 0, 0xA);
    *(int *)(arg0 + 0x98) = *(int *)(*(char **)(arg0 + 0x24) + 0x10);
    *(unsigned char *)(p + 0x6A) = 0;
    *(unsigned char *)(p + 0x2E) = 1;
    *(unsigned char *)(arg0 + 0x7F) = 0;
    *(unsigned char *)(arg0 + 0x31) = 1;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x1000) & 0xFFB8;
    func_L02_003D1E40(arg0);
    *(short *)(p + 0x2FC) = arg1;
    *(short *)(p + 0x88) = 0;
    if (*(short *)(p + 0x50) != 0) {
        *(short *)(p + 0x50) = 0;
        func_316150(arg0, *(unsigned char *)(p + 0x54), *(unsigned char *)(p + 0x55),
                    *(unsigned char *)(p + 0x56));
        *(short *)(p + 0x52) = 1;
    }
    if (*(char **)(p + 0x2E4) != 0) {
        int a, b, c;

        func_316180(arg0, &a, &b, &c);
        func_316150(*(char **)(p + 0x2E4), a, b, c);
    }
}

/* Rebuilds the actor's shared effect record after the sub-object has gone. */
INCLUDE_ASM("asm/levels", func_L02_003D2390);

/* Re-arms the actor, optionally restarting its sub-object. */
INCLUDE_ASM("asm/levels", func_L02_003D2488);

INCLUDE_ASM("asm/levels", func_L02_003D25C8);
