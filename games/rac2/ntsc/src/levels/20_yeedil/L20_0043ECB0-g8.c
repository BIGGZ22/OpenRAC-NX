#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern int D_L20_001B1FD8 __attribute__((sda));

extern float D_L20_001B20B0 __attribute__((sda));

extern float D_L20_001B2110 __attribute__((sda));

extern float D_L20_001B2114 __attribute__((sda));

extern float D_L20_001B2118 __attribute__((sda));

extern float D_L20_001B211C __attribute__((sda));

extern float D_L20_001B213C __attribute__((sda));

extern float D_L20_001B2140 __attribute__((sda));

extern float D_L20_001B2144 __attribute__((sda));

extern float D_L20_001B2148 __attribute__((sda));

extern void func_002B60B0(char *, float, float, float, float);

extern void func_00282CC0(char *, char *, char *);

extern void func_00283098(char *, int *, char *);

void func_L20_00442140(char *arg0) {
    func_002B60B0(arg0, D_L20_001B2114 * 0.000277777785f,
                  D_L20_001B2110 * 0.0166666675f,
                  D_L20_001B2118 * 4.84813745e-06f,
                  D_L20_001B211C * 0.000290888245f);
}

void func_L20_00442740(char *arg0, float f) {
    char *state = *(char **)(arg0 + 0x68);

    func_002B60B0(arg0, D_L20_001B2140 * 0.000277777785f,
                  D_L20_001B213C * 0.0166666675f,
                  D_L20_001B2148 * 4.84813745e-06f,
                  D_L20_001B2144 * 0.000290888245f);
    *(float *)(state + 0x84) = f;
}

void func_L20_00444C18(char *arg0, char *arg1) {
    func_00283098(arg1, &D_L20_001B20B0, arg0 + 0xC0);
    func_00282CC0(arg1, arg1, arg0 + 0x10);
}

extern void func_L00_002EC608(char *, int);

extern void func_L00_002EC970(TI128 *, char *, int, int, float, float);

void func_L20_0043F510(char *arg0) {
    TI128 tmp;
    char *state = *(char **)(arg0 + 0x68);

    func_L00_002EC608(arg0 + 0x10, D_L20_001B1FD8);
    tmp = *(TI128 *)(arg0 + 0x10);
    func_L00_002EC970(&tmp, state + 0x10, D_L20_001B1FD8, 3,
                      *(float *)(state + 0x74), 0.25f);
}

extern float D_L20_001B2024 __attribute__((sda));

extern void func_002B0990(void *, void *, int, int, float);

void func_L20_00440150(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    unsigned short n = *(unsigned short *)(arg0 + 0x96);

    if (n <= 0xFFFE) {
        *(unsigned short *)(arg0 + 0x96) = n + 1;
    }
    func_002B0990(arg0, state + 0x2B0, 2, 1, D_L20_001B2024);
}

extern float D_L20_001B2090 __attribute__((sda));

extern int D_L20_001B2094 __attribute__((sda));

extern void func_00282F20(char *, char *, float);

extern void func_L20_00443B60(char *, char *, char *, float *);

extern void func_L20_0044B100(char *, char *, char *, char *, int, float, int);

/* func_L20_00444088 is defined in L20_00444088-g8.c (it needs its own flags). */

extern float D_L20_001B1FD0 __attribute__((sda));

extern float D_L20_001B1FD4;

extern int D_L20_001B1FC0 __attribute__((sda));

extern int D_L20_001B1FC4 __attribute__((nosda));

extern int D_001B1FC4 __attribute__((nosda));

extern int D_L20_001B1FC8 __attribute__((nosda));

extern int D_L20_001B1FCC __attribute__((nosda));

extern char D_L20_001A8F20[];

extern float func_002A7878(float, float);

extern int func_002A7820(int, int);

extern void func_002BDBC0(char *, char *, int, int, int, float);

void func_L20_0043F460(char *arg0) {
    char *s0 = arg0;
    float f20;
    int r;
    int v[2];

    f20 = func_002A7878(D_L20_001B1FD0, D_L20_001B1FD4) * 210000.0f;
    r = func_002A7820(0x14, 0x50);
    if (*(short *)(s0 + 0xAA) == 0xB5A) {
        v[0] = D_L20_001B1FC0;
        v[1] = D_001B1FC4;
    } else {
        v[0] = D_L20_001B1FC8;
        v[1] = D_L20_001B1FCC;
    }
    func_002BDBC0(s0 + 0x10, D_L20_001A8F20, D_L20_001B1FC0, D_L20_001B1FC4, r, f20);
}
