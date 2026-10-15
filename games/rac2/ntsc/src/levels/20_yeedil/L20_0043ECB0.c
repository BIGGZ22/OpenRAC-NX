#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L20_0043ECB0);

INCLUDE_ASM("asm/levels", func_L20_0043F318);

INCLUDE_ASM("asm/levels", func_L20_0043F460);

INCLUDE_ASM("asm/levels", func_L20_0043F570);

INCLUDE_ASM("asm/levels", func_L20_0043FC70);

INCLUDE_ASM("asm/levels", func_L20_0043FFF0);

INCLUDE_ASM("asm/levels", func_L20_00440198);

INCLUDE_ASM("asm/levels", func_L20_00440398);

extern void func_002A7750(int, int, int);

void func_L20_00440460(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(unsigned short *)(state + 0x178) = 0;
    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_002A7750(arg0, 6, 3);
}

extern void func_00282CC0(char *, char *, char *);
extern void func_00282D30(char *, char *, float);
extern void func_002A8C28(char *, char *, int, int, float, float, float, int,
                          int);
extern int func_00282948(char *);
extern void func_0029F338(char *);

void func_L20_004404B8(char *arg0) {
    char *q = arg0 + 0x10;
    char *state = *(char **)(arg0 + 0x68);

    func_00282CC0(q, q, state);
    *(float *)(state + 8) = *(float *)(state + 8) - *(float *)(state + 0x14);
    func_00282D30(state, state, *(float *)(state + 0x10));
    func_002A8C28(*(char **)(arg0 + 0xB8), q, 0x10001, 0,
                  *(float *)(state + 0x18), 3.0f, 1.0f, 1, 0);
    if (func_00282948(state + 0x1C) != 0) {
        func_0029F338(arg0);
    }
}

extern void func_0027DDC0(int, int, int, int);

void func_L20_00440570(void) {
    func_0027DDC0(0, 0, 0, 0x80);
}

INCLUDE_ASM("asm/levels", func_L20_00440598);

INCLUDE_ASM("asm/levels", func_L20_00441CB8);

INCLUDE_ASM("asm/levels", func_L20_004421A8);

INCLUDE_ASM("asm/levels", func_L20_004424A0);

INCLUDE_ASM("asm/levels", func_L20_00442550);

extern void func_002A8C28(char *, char *, int, int, float, float, float, int,
                          int);

extern float func_00282E80(char *, char *);

extern void func_002B50E0(char *, char *, float);

extern void func_L01_00321178(char *, char *);

void func_L20_004427C8(char *arg0, char *arg1, float f) {
    char *state = *(char **)(arg0 + 0x68);

    func_002A8C28(arg0, arg0 + 0x10, 0x04010000, 0,
                  *(float *)(state + 0x50) + 0.5f, 50.0f, 1.0f, 1, 0);
    if (func_00282E80(arg0 + 0x10, arg1) <= *(float *)(state + 0x84)) {
        func_002B50E0(arg0, arg1, f);
    } else {
        func_L01_00321178(arg0, arg1);
    }
}

INCLUDE_ASM("asm/levels", func_L20_00442890);

INCLUDE_ASM("asm/levels", func_L20_004429F0);

extern int func_002A7820(int, int);

extern int func_L20_004433B0(char *);

extern int func_002A77E0(int);

void func_L20_00442C10(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int v = *(int *)(state + 0x17F4);
    int r;

    if (v == 2) goto other;
    if (v >= 3) goto other;
    if (v != 1) goto other;
    r = func_L20_004433B0(arg0);
    *(int *)(state + 0x17FC) = r;
    if (r == 0) {
        func_002A7750(arg0, 0xC, 4);
    } else {
        func_002A7750(arg0, 4, 2);
    }
    goto done;
other:
    r = func_002A77E0(2) + 2;
    *(int *)(state + 0x17F8) = r;
    *(int *)(state + 0x1810) = func_002A7820(0xB4, 0x168);
    func_002A7750(arg0, 3, 1);
done:
    *(int *)(state + 0x17F4) = *(unsigned char *)(arg0 + 0x95);
}

INCLUDE_ASM("asm/levels", func_L20_00442CD8);

INCLUDE_ASM("asm/levels", func_L20_00442E68);

extern int func_002ABB78(char *);

extern void func_L02_003A6F78(char *);

extern void func_L20_00449A78(char *);

void func_L20_00442F30(char *arg0) {
    if (func_002ABB78(arg0) != 0) {
        func_L02_003A6F78(arg0);
    } else if (*(short *)(arg0 + 0xAA) == 0x12D3) {
        func_L20_00449A78(arg0);
    }
}

INCLUDE_ASM("asm/levels", func_L20_00442F80);

INCLUDE_ASM("asm/levels", func_L20_004431E0);

extern char *D_L20_001B2F9C __attribute__((sda));
extern int func_L03_003CA138(char *);
INCLUDE_ASM("asm/levels", func_L20_004499E8);

INCLUDE_ASM("asm/levels", func_L20_00443328);
INCLUDE_ASM("asm/levels", func_L20_004433B0);

INCLUDE_ASM("asm/levels", func_L20_00443508);

INCLUDE_ASM("asm/levels", func_L20_004435A0);

INCLUDE_ASM("asm/levels", func_L20_00443790);

INCLUDE_ASM("asm/levels", func_L20_00443908);

INCLUDE_ASM("asm/levels", func_L20_00443A10);

INCLUDE_ASM("asm/levels", func_L20_00443B60);

INCLUDE_ASM("asm/levels", func_L20_00443CF0);


INCLUDE_ASM("asm/levels", func_L20_00443D58);

INCLUDE_ASM("asm/levels", func_L20_00443DF0);

INCLUDE_ASM("asm/levels", func_L20_00444018);

INCLUDE_ASM("asm/levels", func_L20_00444088);

INCLUDE_ASM("asm/levels", func_L20_00444130);

INCLUDE_ASM("asm/levels", func_L20_00444138);

INCLUDE_ASM("asm/levels", func_L20_004441D8);

INCLUDE_ASM("asm/levels", func_L20_004448A8);

extern float func_00283248(float, float);

extern float func_00283BE0(float, float);

void func_L20_00444AA8(char *arg0, float *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    float f;

    f = func_00283248(arg1[0] - *(float *)(arg0 + 0x10),
                      arg1[1] - *(float *)(arg0 + 0x14));
    f = func_00283BE0(f, *(float *)(arg0 + 0xF8));
    if (f > 1.0471977f) {
        f = 1.0471977f;
    } else if (f < -1.0471977f) {
        f = -1.0471977f;
    }
    *(float *)(state + 0x2B8) = f;
}

INCLUDE_ASM("asm/levels", func_L20_00444B48);

extern void func_L00_002E6EF8(void (*)(void), int);

void func_L20_00444C68(int arg0) {
    func_002A7750(arg0, 0x12, -1);
    func_L00_002E6EF8(func_L20_00440570, arg0);
}

INCLUDE_ASM("asm/levels", func_L20_00444CA8);

INCLUDE_ASM("asm/levels", func_L20_004450D8);

INCLUDE_ASM("asm/levels", func_L20_004451C0);

INCLUDE_ASM("asm/levels", func_L20_00445738);

INCLUDE_ASM("asm/levels", func_L20_00445890);

INCLUDE_ASM("asm/levels", func_L20_00446E70);

INCLUDE_ASM("asm/levels", func_L20_00446FA8);

INCLUDE_ASM("asm/levels", func_L20_004473B0);

INCLUDE_ASM("asm/levels", func_L20_00447640);

INCLUDE_ASM("asm/levels", func_L20_00447748);

extern char D_L20_002AB980[];

extern void func_L04_003A21F8();

void func_L20_00447908(void) {
    func_L04_003A21F8(D_L20_002AB980, 6);
}

INCLUDE_ASM("asm/levels", func_L20_00447930);

INCLUDE_ASM("asm/levels", func_L20_00448858);

INCLUDE_ASM("asm/levels", func_L20_00448A18);
