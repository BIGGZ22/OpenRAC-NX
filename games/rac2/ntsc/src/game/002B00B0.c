#include "include_asm.h"

/* Measured partial C views; original types and object ownership are unknown. */
typedef unsigned char QwenRecovery_00beefd949f6_AbiByte15;

typedef unsigned long long QwenRecovery_00beefd949f6_AbiFlag15;

extern void func_002AFFD8(QwenRecovery_00beefd949f6_AbiByte15 *input, float *output, QwenRecovery_00beefd949f6_AbiFlag15 mode);

extern void func_002AFFD8_1(void *a, void *b, int c);

extern void func_00282F20_14(float f, void *a, void *b);

extern void func_00282CC0_35(char *target, char *first, char *local);

extern int D_L00_001A8FF0_3 __attribute__((sda));

extern int D_L00_001A8FF4 __attribute__((sda));

extern char D_L00_001B1B10[] __attribute__((sda));

extern char D_L00_001A9000[] __attribute__((sda));

extern float func_00282E48_1(char *a, char *b);

extern void func_00282F20_6(void *, float);

extern void func_00282DD8_2(void *, void *, void *);

extern void func_00282D30_12(void *, void *, float);

extern float func_00282DF0_1(void *);

extern float func_00282C10(float);

extern void func_00283958(void *, void *);

extern void func_00283738(void *, void *, void *);

extern void func_00282AB0_1(void *, void *, int);

extern float func_00283CE0_2(int);

extern float func_002A7940_1(void);

extern float func_002A7878_3(float, float);

extern float func_00283180_2(float);

extern float func_00283198_4(float);

extern void func_00283098_3(float *dst, float *src, char *table);

extern void func_00282CC0_11(float *dst, float *src, int arg);

extern int D_L00_001A8FF0_1 __attribute__((sda));

extern void func_00273730(char *self, int a1, int a2, int a3, char *p4, char *p5,
                    float *buf, int b3, float fa, float fb);

typedef int s32;

typedef unsigned char u8;

extern u8 D_00139648;

extern unsigned char D_19B278[];

extern int F9a90bcc4_FUN_002A77E0(int count);

void func_002B00B0(QwenRecovery_00beefd949f6_AbiByte15 *param_1, float *output, QwenRecovery_00beefd949f6_AbiFlag15 mode)
{
    func_002AFFD8(param_1 + 0x10, output, mode);
    return;
}

INCLUDE_ASM("asm/game", func_002B00D0);

/* If the linked global word is zero, stores (second_float_field - f) into the first object's float field; otherwise runs a three-call path over a 16-byte local buffer. */
void func_002B00D8(void *p, float *a1, float *a2, float f)
{
    float buf[4];

    if (D_L00_001A8FF0_3 == 0) {
        a1[2] = a2[2] - f;
    } else {
        func_002AFFD8_1(p, buf, 0);
        func_00282F20_14(f, buf, buf);
        func_00282CC0_35((char *)a1, (char *)a2, (char *)buf);
    }
}

void func_002B0158(char *object, float *first, float *second, float amount) {
    func_002B00D8(object + 0x10, first, second, amount);
}

INCLUDE_ASM("asm/game", func_002B0178);

INCLUDE_ASM("asm/game", func_002B0358);

/* Returns a scaled length when a global flag is set, otherwise refreshes a bound data object and forwards the raw helper result. */
float func_002B0360(char *p)
{
    float v;

    if (D_L00_001A8FF4 == 0) {
        v = func_00282E48_1(p, D_L00_001B1B10);
    } else {
        v = 100.0f - func_00282E48_1(p, D_L00_001A9000);
    }
    return v;
}

extern void func_00282D30(void *, void *, float);
extern void func_002B0408(void *, void *, void *);

void func_002B03B8(void *first, void *second, void *third) {
    float vector[4];
    func_00282D30(vector, second, -1.0f);
    func_002B0408(first, vector, third);
}

/* Builds a 4-float value from a unit seed and a 0.5f-scaled blend against a region 32 bytes into the first argument, squares a scalar returned by a fourth callee, stores the negated complement of that square into a fifth word, converts the value into a 48-byte record and copies that record into the third argument when it is non-null. */
void func_002B0408(void *a0, void *a1, void *a2)
{
    float buf[12];
    float m[4];
    float q[4];
    float x;

    func_00282F20_6(q, 1.0f);
    func_00282DD8_2(m, q, (char *)a0 + 32);
    func_00282D30_12(m, m, 0.5f);
    x = func_00282DF0_1(m);
    m[3] = -func_00282C10(1.0f - x * x);
    func_00283958(m, buf);
    func_00283738(a0, buf, a0);
    if (a2 != 0)
        func_00282AB0_1(a2, buf, 48);
}

INCLUDE_ASM("asm/game", func_002B04D8);

extern void func_002AFDD8(char *a0, char *a1, char *a2, char *a3);
extern float func_00283248(float a, float b);

typedef int V4_002B04E0 __attribute__((mode(TI)));

float func_002B04E0(char *a0, float *src, char *third)
{
    float local[4];

    *(V4_002B04E0 *)local = *(V4_002B04E0 *)src;
    func_002AFDD8(a0, (char *)local, (char *)local, third);
    return func_00283248(local[0], local[1]);
}

INCLUDE_ASM("asm/game", func_002B0518);

INCLUDE_ASM("asm/game", func_002B0520);

INCLUDE_ASM("asm/game", func_002B0668);

INCLUDE_ASM("asm/game", func_002B08A0);

/* Calls one float-conversion helper on (a0 %% a1) and again on a1, then returns 2*pi times the quotient minus pi. */
float func_002B08A8(int a0, int a1)
{
    float r = func_00283CE0_2(a0 % a1);
    float s = func_00283CE0_2(a1);

    return 2.0f * r * 3.1415927f / s - 3.1415927f;
}

extern float func_002B08A8(int, int);
extern float func_00283198(float);
extern void func_00283D38(int, int, float);
void func_002B0910(int a0, int a1, int a2, int a3)
{
    float v = func_002B08A8(a0, a1);
    v = func_00283198(v);
    func_00283D38(a2, a3, v * 0.5f + 0.5f);
}

INCLUDE_ASM("asm/game", func_002B0968);

INCLUDE_ASM("asm/game", func_002B0990);

INCLUDE_ASM("asm/game", func_002B0A08);

void func_002B0A18(int first, int second)
{
    extern int func_00288FB8(int);
    extern void func_0029CC98(int, int);
    func_0029CC98(func_00288FB8(first), second);
}

extern void func_0029CC50(int, int, int);

void func_002B0A48(int first, int second, int third) {
    func_0029CC50(first, second, third);
}

/* Walks selected mask bits, evaluates three stored curve components, optionally transforms the vector, and dispatches one object operation per selected bit. */
void func_002B0A68(char *self, int mask, int arg2,
                                        unsigned char arg3,
                                        float f0, float f1, float f2, float f3)
{
    float buf[3];
    float v;
    float k;
    float fa;
    float fb;
    int i;

    for (i = 0; i < 15; i++) {
        if (((mask >> i) ^ 1) & 1)
            continue;
        v = func_002A7940_1();
        k = func_002A7878_3(f0, f1) * (1.0f / 60.0f);
        buf[0] = func_00283180_2(v) * k;
        buf[1] = func_00283198_4(v) * k;
        buf[2] = 0.0f;
        buf[2] = func_002A7878_3(f2, f3) * (1.0f / 60.0f);
        if (D_L00_001A8FF0_1 != 0)
            func_00283098_3(buf, buf, self + 192);
        func_00282CC0_11(buf, buf, arg2);
        fa = func_002A7878_3(90.0f, 270.0f);
        fb = func_002A7878_3(10.0f, 20.0f);
        func_00273730(self, 2, *(short *)(self + 170), i, self + 16, self + 240,
                buf, arg3, fa, fb);
    }
}

INCLUDE_ASM("asm/game", func_002B0C18);

INCLUDE_ASM("asm/game", func_002B0C28);

INCLUDE_ASM("asm/game", func_002B0CE0);

s32 func_002B0D60(void) { return D_00139648; }

extern int func_002B0DB8(void);
int func_002B0D70(void) {
    int count = func_002B0DB8();
    int diff = count - func_002B0D60();
    if (diff < 0) diff = 0;
    if (diff > 40) diff = 40;
    return diff;
}

int func_002B0DB8(void)
{
    int count = 0;
    int i;

    for (i = 0; i < 0x1C; i++) {
        int j;

        for (j = 0; j < 4; j++) {
            if (D_19B278[i * 4 + j] != 0)
                count++;
        }
    }
    if (count < 0)
        count = 0;
    if (count > 0x28)
        count = 0x28;
    return count;
}

INCLUDE_ASM("asm/game", func_002B0E28);

INCLUDE_ASM("asm/game", func_002B0E90);

INCLUDE_ASM("asm/game", func_002B0ED8);

INCLUDE_ASM("asm/game", func_002B1350);

int func_002B1A80(u8 *owner, int b, int *outIndex,
                                       float *o0, float *o1, float *o2, float *o3,
                                       float *o4, float *o5, float *o6)
{
    int n;
    int count;
    int i;
    int k;
    int left;
    u8 *p;

    n = 0;
    count = *(u8 *)(*(int *)(owner + 36) + 12);
    if (count != 0) {
        int *t = (int *)(*(int *)(owner + 36) + 72);
        u8 *q;
        left = count;
        do {
            q = *(u8 **)(*t + 20);
            if (q != 0 && *q == b)
                n++;
            t++;
            left--;
        } while (left != 0);
    }

    if (n != 0)
        goto second;

    return 0;

found:
    *o0 = *(float *)(p + 8) * 0.016666668f;
    *o1 = *(float *)(p + 12) * 0.016666668f;
    *o2 = *(float *)(p + 16) * 0.00027777778f;
    *o3 = *(float *)(p + 20) * 0.00027777778f;
    *o6 = *(float *)(p + 4) * 0.00027777778f;
    *o4 = *(float *)(p + 24);
    *o5 = *(float *)(p + 28);
    *outIndex = i;
    return 1;

second:
    k = F9a90bcc4_FUN_002A77E0(n);
    for (i = 0; i < *(u8 *)(*(int *)(owner + 36) + 12); i++) {
        char *tbl = (char *)(*(int *)(owner + 36) + 72);
        char *e = *(char **)(tbl + i * 4);
        p = *(u8 **)(e + 20);
        if (p == 0 || *p != b)
            continue;
        if (k == 0)
            goto found;
        k--;
    }
    return 0;
}
