#include "include_asm.h"

extern int D_L20_001B20B0 __attribute__((sda));

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
