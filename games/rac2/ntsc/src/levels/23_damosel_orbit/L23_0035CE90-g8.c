#include "include_asm.h"

extern float D_L23_001AFAA0 __attribute__((sda));

extern float D_L23_001B13C8 __attribute__((sda));

extern void func_3242D0(int, int, int);

extern void func_L23_004160B0(char *);

extern void func_31AF00(int, int, char *);

extern void func_2F8EA8(char *, int, float);

extern float func_2F91F0(float, float);

void func_L23_003C1AE8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(state + 0x371) == 0)
        func_31AF00(*(int *)(state + 0x2A0), 2, state + 0x370);
    func_2F8EA8(state + 0x380, 2, *(float *)(state + 0x3B0));
    *(float *)(state + 0x3B0) = func_2F91F0(*(float *)(state + 0x3B0),
                                            D_L23_001AFAA0 * 0.000290888245f);
}

extern int D_L23_001B36B0;

extern float D_L23_001AFA34 __attribute__((sda));

extern char D_00189EA0[];

extern void func_2F8190(char *out, char *in, char *base);

extern void func_2F8440(char *out, char *in, float scale);

extern float func_2F8268(char *out, char *in);

void func_L23_00415FC8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    unsigned char kind = *(unsigned char *)(arg0 + 0x20);

    if (kind == 0) {
        *(unsigned short *)(arg0 + 0x34) |= 0x100;
        *(unsigned short *)(arg0 + 0x32) = 0xFF;
        *(unsigned char *)(arg0 + 0x30) = 0xFF;
        *(float *)(state + 0x20) = 2000.0f;
        *(short *)(state + 0x24) = 2000;
        *(short *)(arg0 + 0x7C) = 0x3FF;
        func_3242D0(arg0, 1, 0);
        *(unsigned short *)(arg0 + 0x34) |= 0x4000;
    } else if (kind == 1) {
        float f = D_L23_001B13C8;

        if (f != 0.0f) {
            *(float *)(state + 0x20) = f;
            *(short *)(state + 0x24) = f;
        }
        func_3242D0(arg0, 2, 0);
    }
    func_L23_004160B0(arg0);
}

INCLUDE_ASM("asm/levels", func_L23_003C19E0);
