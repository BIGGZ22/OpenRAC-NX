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

extern int D_L23_001B36B0;

extern float D_L23_001AFAB4 __attribute__((sda));

extern char D_00189EA0[];

extern void func_00282CF0(char *out, char *a, char *b);

extern void func_00282F20(char *out, char *in, float length);

extern float func_00282DB0(char *a, char *b);

/* in state 8, with the flag at 0x001B36B0 clear, the object drawn and the half-word at +0x324 of
   its state set: compares the normalised offset from D_00189EA0 with the vector at +0xC0 of the
   object D_00189EA0 holds at +0x2210, and within the threshold hands back the float at +0x27C */
int func_L23_003C19E0(char *arg0, float *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    char dir[16];

    if (*(unsigned char *)(arg0 + 0x20) == 8 && D_L23_001B36B0 == 0 &&
        *(unsigned char *)(arg0 + 0x31) != 0 && *(short *)(state + 0x324) != 0) {
        func_00282CF0(dir, arg0 + 0x10, D_00189EA0);
        func_00282F20(dir, dir, 1.0f);
        if (func_00282DB0(dir, (char *)(*(int *)(D_00189EA0 + 0x2210) + 0xC0)) <= D_L23_001AFAB4) {
            *arg1 = *(float *)(*(char **)(arg0 + 0x68) + 0x27C);
            return 1;
        }
    }
    return 0;
}
