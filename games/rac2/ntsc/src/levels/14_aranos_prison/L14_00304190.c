#include "include_asm.h"
#include "common.h"

extern void func_321F58();
extern char *func_328768();
extern void func_328DA8();
extern void func_3322F0();
extern void func_3DECE0();
extern void func_3DF6A0();
extern float func_3E22A8();
extern void func_2F5C38();
extern void func_2F5C68();
extern void func_2F6498();
extern void func_2F5CC0();
extern void func_31A618();
extern void func_4209A0();
extern int func_420C68();
extern float func_2F6C80(float a, float b);
extern float func_2F6240(float a);
extern void func_2F5F18(void *dst, void *src, float s);
extern void func_2F5D68(void *dst, void *src, void *mtx);
extern void func_3910E0(char *a0, char *a1, int a2, int a3, int t0, int t1, int t2,
                        float f12, float f13, float f14, float f15, float f16);

extern char D_0018A0E0[];
extern char D_L14_00237B00[];

INCLUDE_ASM("asm/levels", func_L14_00304190);

/* func_L14_00304280 lives in the -g8 sibling file (it reaches a global through $gp). */

INCLUDE_ASM("asm/levels", func_L14_00304588);

INCLUDE_ASM("asm/levels", func_L14_00328E70);

void func_L14_00328E90(char *arg0, char *arg1, char *arg2) {
    char buf[16];
    char *s1 = func_328768(arg0);

    if (s1 != 0) {
        func_2F5C68(buf, arg0 + 0x10, arg1);
        func_328DA8(s1, buf, arg2, arg0 + 0xF0);
    }
}

INCLUDE_ASM("asm/levels", func_L14_0035F510);

INCLUDE_ASM("asm/levels", func_L14_00390DF8);

INCLUDE_ASM("asm/levels", func_L14_00390EC0);

void func_L14_00390FE8(char *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);
    char *base = D_L14_00237B00;
    char *base2 = base + 0x400;
    int i;

    for (i = 0; i < 4; i++) {
        func_3910E0(arg0, (char *)(((*(int *)(s0 + 0xDC) + i) << 8) + (unsigned int)base),
                    *(int *)(s0 + 0x38), *(int *)(s0 + 0x40),
                    *(int *)(s0 + 0x60), *(int *)(s0 + 0x44), *(int *)(s0 + 0x64),
                    *(float *)(s0 + 0x50), *(float *)(s0 + 0x70), *(float *)(s0 + 0x54),
                    *(float *)(s0 + 0x74), 0.9f);
        func_3910E0(arg0, (char *)(((*(int *)(s0 + 0xDC) + i) << 8) + (unsigned int)base2),
                    *(int *)(s0 + 0x38), *(int *)(s0 + 0x48),
                    *(int *)(s0 + 0x68), *(int *)(s0 + 0x4C), *(int *)(s0 + 0x6C),
                    *(float *)(s0 + 0x58), *(float *)(s0 + 0x78), *(float *)(s0 + 0x5C),
                    *(float *)(s0 + 0x7C), 0.1f);
    }
}

void func_L14_00391B58(char *arg0) {
    char buf[16];

    func_2F5CC0(buf, arg0);
    func_2F5C38(arg0 + 0x30, arg0 + 0x30, buf);
}

INCLUDE_ASM("asm/levels", func_L14_00399E28);

INCLUDE_ASM("asm/levels", func_L14_0039A240);

INCLUDE_ASM("asm/levels", func_L14_0039F820);

int func_L14_003C9548(char *arg0) {
    return (arg0[0x95] ^ 1) == 0;
}

INCLUDE_ASM("asm/levels", func_L14_003C9658);

INCLUDE_ASM("asm/levels", func_L14_003C96E0);

INCLUDE_ASM("asm/levels", func_L14_003CB258);

INCLUDE_ASM("asm/levels", func_L14_003CB4D8);

INCLUDE_ASM("asm/levels", func_L14_003CB608);

INCLUDE_ASM("asm/levels", func_L14_003CB868);

INCLUDE_ASM("asm/levels", func_L14_003D18C8);

INCLUDE_ASM("asm/levels", func_L14_003D54A8);

INCLUDE_ASM("asm/levels", func_L14_003DDD38);

int func_L14_003DE2D8(char *arg0) {
    int *p = (int *)(*(int *)(arg0 + 0x68) + 0xE0);
    int count = 0;
    int i;

    for (i = 3; i >= 0; i--) {
        if (*p != 0) {
            count++;
        }
        p++;
    }
    return count;
}

INCLUDE_ASM("asm/levels", func_L14_003DE360);

void func_L14_003DE488(char *arg0) {
    char **p = (char **)(*(int *)(arg0 + 0x68) + 0xE0);
    int i;

    for (i = 3; i >= 0; i--) {
        char *v = *p;

        if (v != 0 && *(int *)(*(int *)(v + 0x68) + 0x2E0) != 0) {
            func_3322F0(v, 1);
        }
        p++;
    }
}

INCLUDE_ASM("asm/levels", func_L14_003DE4E8);

int func_L14_003DE8E8(unsigned char *arg0) {
    return (unsigned int)(arg0[0x20] - 2) < 2;
}

void func_L14_003DE8F8(char *arg0) {
    *(int *)(*(int *)(arg0 + 0x68) + 0x84) = 1;
}

void func_L14_003DE990(char *arg0) {
    func_321F58(arg0, 2, -1);
}

INCLUDE_ASM("asm/levels", func_L14_003DE9B0);

INCLUDE_ASM("asm/levels", func_L14_003DECE0);

/* func_L14_003DF100 lives in the -g8 sibling file (its calls pass floats, so it needs real prototypes). */

INCLUDE_ASM("asm/levels", func_L14_003DF200);

INCLUDE_ASM("asm/levels", func_L14_003DF440);

INCLUDE_ASM("asm/levels", func_L14_003DF540);

void func_L14_003DF650(unsigned char *arg0) {
    if (arg0[0x20] == 0) {
        func_3DECE0();
    }
    *(int *)(arg0 + 0x64) = (int)func_3DF6A0;
}

void func_L14_003DF690(char *arg0, int arg1) {
    *(int *)(*(int *)(arg0 + 0x68) + 0x7C) = arg1;
}

INCLUDE_ASM("asm/levels", func_L14_003DF6A0);

/* func_L14_003DF7F8 is defined in L14_00304190-g8.c (this unit has no $gp access for it). */

void func_L14_003DFA10(char *arg0, char *arg1) {
    char buf[16];
    char *v = *(char **)(arg0 + 0x68);

    func_2F5C38(buf, arg0 + 0xC0, arg0 + 0x10);
    func_4209A0(*(int *)(v + 0x0), buf, arg1);
}

INCLUDE_ASM("asm/levels", func_L14_003DFA60);

void func_L14_003E1DD0(char *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);
    unsigned char st = *(unsigned char *)(arg0 + 0x20);

    switch (st) {
    case 0:
        *(float *)(s0 + 0x4) = *(float *)(arg0 + 0x18);
        *(int *)(s0 + 0x8) = 0;
        func_321F58(arg0, 1, -1);
        break;
    case 1:
        *(float *)(arg0 + 0xF8) = func_2F6C80(*(float *)(arg0 + 0xF8), *(float *)(s0 + 0x0));
        *(float *)(s0 + 0x8) = func_2F6C80(*(float *)(s0 + 0x8), *(float *)(s0 + 0xC));
        *(float *)(arg0 + 0x18) = *(float *)(s0 + 0x4)
                                  + *(float *)(s0 + 0x10) * func_2F6240(*(float *)(s0 + 0x8));
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L14_003DE310);

INCLUDE_ASM("asm/levels", func_L14_003E1E78);

void func_L14_003E2238(char *arg0) {
    float f = func_3E22A8(arg0);

    func_2F6498(arg0 + 0xC0, arg0 + 0xF0);
    ((void (*)(float *, float *, float))func_2F5CC0)((float *)(arg0 + 0xC0),
                                                     (float *)(arg0 + 0xC0), f);
    ((void (*)(float *, float *, float))func_2F5CC0)((float *)(arg0 + 0xD0),
                                                     (float *)(arg0 + 0xD0), f);
    func_31A618(arg0);
}

float func_L14_003E22A8(char *arg0) {
    char *v = *(char **)(arg0 + 0x68);
    unsigned int s = *(unsigned char *)(arg0 + 0x20);

    if (s == 4) {
        return (float)*(int *)(v + 0x7C) / (float)*(int *)(v + 0x78);
    }
    if (s == 2) {
        return 1.0f - (float)*(int *)(v + 0x7C) / (float)*(int *)(v + 0x70);
    }
    if (s == 1) {
        return 0.0f;
    }
    return 1.0f;
}

INCLUDE_ASM("asm/levels", func_L14_003E2328);

INCLUDE_ASM("asm/levels", func_L14_003E2688);

float func_L14_003E28E8(float arg0) {
    float f = arg0;

    while (6.2831855f < f) {
        f = f - 6.2831855f;
    }
    while (f < 0.0f) {
        f = f + 6.2831855f;
    }
    return f;
}

INCLUDE_ASM("asm/levels", func_L14_003E2980);

INCLUDE_ASM("asm/levels", func_L14_003E4208);

INCLUDE_ASM("asm/levels", func_L14_003E4A40);

INCLUDE_ASM("asm/levels", func_L14_003E4BA0);

INCLUDE_ASM("asm/levels", func_L14_003E4EA0);

/* func_L14_003E5030 lives in the -g8 sibling file (it reaches a global through $gp). */

INCLUDE_ASM("asm/levels", func_L14_003E50B8);

void func_L14_003E5190(char *arg0) {
    char *v0 = *(char **)(arg0 + 0x68);
    v0 = *(char **)(v0 + 0x30);
    if (v0 != 0) {
        v0 = *(char **)(v0 + 0x68);
        *(int *)(v0 + 0x3C) = 0;
    }
}

void func_L14_003E51B0(char *arg0) {
    char *v0 = *(char **)(arg0 + 0x68);
    char *v1 = *(char **)(v0 + 0x30);

    if (v1 != 0) {
        char *v2 = *(char **)(v1 + 0x68);

        *(int *)(v2 + 0x48) = ((unsigned int)*(s16 *)(v0 + 0x44) << 16)
                              | ((unsigned int)*(s16 *)(v0 + 0x42) << 8)
                              | (unsigned int)*(s16 *)(v0 + 0x40);
        *(int *)(v2 + 0x4C) = ((unsigned int)*(s16 *)(v0 + 0x4A) << 16)
                              | ((unsigned int)*(s16 *)(v0 + 0x48) << 8)
                              | (unsigned int)*(s16 *)(v0 + 0x46);
        *(int *)(v2 + 0x3C) = 1;
        *(int *)(v2 + 0x34) = *(int *)(v0 + 0x18);
    }
}

INCLUDE_ASM("asm/levels", func_L14_003E5218);

extern void func_L00_002E54E0(float);
extern int func_002B38B8(char *);
extern void func_L00_002E6EF8(void (*f)(char *), char *arg);
extern float func_002A7878(float, float);
extern void func_L14_003F0E68(char *);

void func_L14_003F0D50(char *arg0) {
    char *s2 = arg0;
    char *s1 = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) == 0) {
        *(int *)(s1 + 0x10) = 1;
        func_L00_002E54E0(2048.0f);
        *(unsigned char *)(arg0 + 0x20) = 1;
    }
    *(float *)(s1 + 0x0) = *(float *)(s1 + 0x0) + *(float *)(s1 + 0x4);
    *(float *)(s1 + 0x8) = *(float *)(s1 + 0x8) + *(float *)(s1 + 0xC);
    if (*(float *)(s1 + 0x0) >= 8.0f) {
        *(float *)(s1 + 0x0) = *(float *)(s1 + 0x0) - 8.0f;
    }
    if (*(float *)(s1 + 0x8) >= 8.0f) {
        *(float *)(s1 + 0x8) = *(float *)(s1 + 0x8) - 8.0f;
    }
    if (func_002B38B8(s2) != 0) {
        *(int *)(s1 + 0x10) = 0;
    }
    if (*(int *)(s1 + 0x10) != 0) {
        func_L00_002E6EF8(func_L14_003F0E68, s2);
    }
    *(float *)(s1 + 0x4) = func_002A7878(0.003f, 0.005f);
    *(float *)(s1 + 0xC) = func_002A7878(0.001f, 0.0025f);
}

/* Resets the record: the four axis pairs, the flags and the packed word. */
void func_L14_003F0E98(char *a0)
{
    *(long long *)(a0 + 0x70) = 0LL;
    *(int *)(a0 + 0x50) = 0;
    *(int *)(a0 + 0x54) = 0;
    *(float *)(a0 + 0x58) = 1.0f;
    *(int *)(a0 + 0x5C) = 0;
    *(int *)(a0 + 0x60) = 0;
    *(float *)(a0 + 0x64) = 1.0f;
    *(float *)(a0 + 0x68) = 1.0f;
    *(float *)(a0 + 0x6C) = 1.0f;
    *(float *)(a0 + 0x8) = -4.0f;
    *(float *)(a0 + 0x18) = 4.0f;
    *(float *)(a0 + 0x4) = -2.0f;
    *(float *)(a0 + 0x24) = 2.0f;
    *(int *)(a0 + 0x0) = 0;
    *(int *)(a0 + 0x10) = 0;
    *(int *)(a0 + 0x20) = 0;
    *(int *)(a0 + 0x30) = 0;
    *(float *)(a0 + 0xC) = 1.0f;
    *(float *)(a0 + 0x1C) = 1.0f;
    *(float *)(a0 + 0x2C) = 1.0f;
    *(long long *)(a0 + 0x80) = 280993940374112LL;
    *(float *)(a0 + 0x28) = -4.0f;
    *(float *)(a0 + 0x38) = 4.0f;
    *(float *)(a0 + 0x14) = -2.0f;
    *(float *)(a0 + 0x34) = 2.0f;
    *(float *)(a0 + 0x3C) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L14_003F0F38);
