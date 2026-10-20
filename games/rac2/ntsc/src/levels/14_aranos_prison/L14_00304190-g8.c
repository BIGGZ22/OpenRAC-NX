#include "include_asm.h"

extern int D_001A79F0;

extern int D_001A72E0;

extern int D_L14_001B23D0 __attribute__((sda));

extern int D_L14_001B259C __attribute__((sda));

extern char D_L14_00284690[];

extern void func_367708();

extern void func_3674A0();

extern void func_367778();

extern void func_385CB8(unsigned int reg, unsigned long value);

extern void func_L14_00304190();

extern void func_2F5CC0(void *dst, void *src, float s);

extern void func_2F5C38(void *dst, void *src, void *src2);

extern void func_420B10();

extern void func_321F58();

extern void func_3DFA10();

extern int func_002B38B8();

extern int func_3322A0();

extern void func_369AD0();

extern void func_2F5C10();

extern void func_2F5C38(void *dst, void *src, void *src2);

extern void func_2F5D68(void *dst, void *src, void *mtx);

extern void func_2F5F18(void *dst, void *src, float s);

extern float func_2F6C80(float a, float b);

extern float func_2F5B78(float a);

extern void func_317D90();

extern void func_2EE5F0();

extern void func_L14_003DFA60();

extern float D_L14_001B0388 __attribute__((sda));

extern float D_L14_001B038C __attribute__((sda));

extern float D_L14_001B0390 __attribute__((sda));

/* This unit is built with -G8: these two bodies reach a global through $gp. */

void func_L14_00304280(void) {
    func_367708();
    D_L14_001B23D0 = (int)D_L14_00284690;
    if (D_001A79F0 == 0xE) {
        func_L14_00304190();
    } else {
        func_3674A0();
    }
    func_367778();
    func_385CB8(0x47, 0x5360B);
    func_385CB8(0x4E, 0x1000000 | (D_001A72E0 >> 13));
}

INCLUDE_ASM("asm/levels", func_L14_003E5030);

void func_L14_003DF100(char *arg0, int arg1) {
    float a[4];
    float b[4];
    char *v;
    char *p;

    v = *(char **)(arg0 + 0x68);
    func_2F5CC0(a, arg0 + 0xC0, 2.0f);
    func_2F5CC0(b, arg0 + 0xC0, -2.0f);
    if (*(int *)(v + 0x7C) != 0) {
        func_2F5CC0(a, a, -1.0f);
        func_2F5CC0(b, b, -1.0f);
    }
    p = arg0 + 0x10;
    func_2F5C38(a, a, p);
    func_2F5C38(b, b, p);
    func_420B10(*(int *)(v + 0x64), a, p, arg1 == 0);
    func_420B10(*(int *)(v + 0x64), b, p, 0);
}

void func_L14_003DF7F8(char *arg0) {
    char *v = *(char **)(arg0 + 0x68);

    switch (*(unsigned char *)(arg0 + 0x20)) {
    case 0:
        func_321F58(arg0, 1, -1);
        if (*(int *)(v + 0x0) != -1) {
            func_3DFA10(arg0, 0);
        }
        break;
    case 1:
        if (func_002B38B8(arg0) != 0) {
            if (*(int *)(v + 0x0) != -1) {
                func_3DFA10(arg0, 1);
            }
            func_321F58(arg0, 2, -1);
            {
                float s = D_L14_001B0388 * 0.016666668f;

                *(float *)(v + 0xC) = s;
                if (*(int *)(v + 0x8) != 0) {
                    *(float *)(v + 0xC) = -s;
                }
            }
            *(int *)(v + 0x10) = 0;
        }
        break;
    case 2:
        {
            float a[4];
            float b[4];

            if (func_3322A0(arg0) != 0) {
                func_369AD0(0, 0, arg0);
            }
            func_2F5C10(b);
            func_2F5D68(a, arg0 + 0xC0, b);
            func_2F5F18(a, a, *(float *)(v + 0xC));
            func_2F5C38(arg0 + 0x10, arg0 + 0x10, a);
            {
                float old = *(float *)(v + 0xC);
                float m;

                *(float *)(v + 0x10) = *(float *)(v + 0x10) + old;
                if (*(int *)(v + 0x8) != 0) {
                    m = old - D_L14_001B038C * 0.00027777778f;
                } else {
                    m = old + D_L14_001B038C * 0.00027777778f;
                }
                {
                    float x = m * D_L14_001B0390;

                    *(float *)(v + 0xC) = m;
                    *(float *)(arg0 + 0xF0) = func_2F6C80(*(float *)(arg0 + 0xF0), x);
                }
            }
            if (func_2F5B78(*(float *)(v + 0x10)) > 4.0f) {
                func_317D90(arg0);
                return;
            }
        }
        break;
    }
    if (*(unsigned char *)(arg0 + 0x31) != 0 && *(int *)(v + 0x4) != -1) {
        func_2EE5F0(func_L14_003DFA60, arg0);
    }
}
