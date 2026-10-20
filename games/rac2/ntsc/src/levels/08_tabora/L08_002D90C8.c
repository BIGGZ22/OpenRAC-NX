#include "include_asm.h"

typedef unsigned int ti_word __attribute__((mode(TI)));

typedef float f32;

extern void func_2F9E60(void *, void *, f32);

extern void func_2F9DF0(void *, void *, void *);

extern f32 func_2F9FD0(void *, void *);

extern int *D_L08_001FE7C0[];

extern float func_00282E80(void *, void *);

extern float func_L08_0032EB48(ti_word *, int, float);

extern float func_L08_0032EC38(ti_word *, int, float);

extern float func_L08_0032ED98(ti_word *, int, float);

extern char D_L08_002A0D00[];

extern void func_L04_003A21F8(void *, int);

extern void func_L04_003A02A0(void *, int);

extern void func_0029FCC0(void *, int, void *);

INCLUDE_ASM("asm/levels", func_L08_002D90C8);

INCLUDE_ASM("asm/levels", func_L08_00308888);

void func_L08_00329EC0(float value, float *object) {
    object[29] = 0.2f;
    object[28] = value - object[6];
}

f32 func_L08_0032EA60(char *a0, char *a1, char *a2, f32 f0, f32 f1, f32 f2, f32 f3) {
    f32 out[4];
    f32 t[4];
    f32 r;

    func_2F9E60(t, a1, f1);
    func_2F9DF0(out, a2, t);
    func_2F9E60(t, a1 + 0x10, f2);
    func_2F9DF0(out, out, t);
    func_2F9E60(t, a1 + 0x20, f3);
    func_2F9DF0(out, out, t);
    r = func_2F9FD0(a0, out);
    if (f0 < r) f0 = r;
    return f0;
}

INCLUDE_ASM("asm/levels", func_L08_0032EB48);

INCLUDE_ASM("asm/levels", func_L08_0032EC38);

INCLUDE_ASM("asm/levels", func_L08_0032ED98);

float func_L08_0032EED8(ti_word *object, int index, float minimum) {
    int *list;
    int i;
    if (index == -1) return minimum;
    list = D_L08_001FE7C0[index];

    for (i = 0; i < *list; i++) {
        float value = func_00282E80((char *)(list + 4) + i * 0x10, object);
        if (minimum < value) minimum = value;
    }
    return minimum;
}

float func_L08_0032EF80(ti_word *input, int initial, int first, int second, int third) {
    ti_word value = *input;
    float result;
    result = func_L08_0032EED8(&value, initial, 0.0f);
    result = func_L08_0032EB48(&value, first, result);
    result = func_L08_0032EC38(&value, second, result);
    return func_L08_0032ED98(&value, third, result);
}

INCLUDE_ASM("asm/levels", func_L08_0032F898);

INCLUDE_ASM("asm/levels", func_L08_003CC750);

INCLUDE_ASM("asm/levels", func_L08_003CCC30);

INCLUDE_ASM("asm/levels", func_L08_003CCD80);

INCLUDE_ASM("asm/levels", func_L08_003CFE68);

void func_L08_003CFFE8(void) {
    func_L04_003A21F8(D_L08_002A0D00, 7);
}

void func_L08_003D0010(void) {
    func_L04_003A02A0(D_L08_002A0D00, 7);
}

void func_L08_003D0040(void *object) {
    void **parts = *(void ***)((char *)object + 0x68);
    if (parts[0]) func_0029FCC0(object, 0, (char *)parts[0] + 0x10);
    if (parts[1]) func_0029FCC0(object, 1, (char *)parts[1] + 0x10);
}

INCLUDE_ASM("asm/levels", func_L08_003D00C8);

INCLUDE_ASM("asm/levels", func_L08_003D0508);

INCLUDE_ASM("asm/levels", func_L08_003D2F50);

INCLUDE_ASM("asm/levels", func_L08_003D3D80);

INCLUDE_ASM("asm/levels", func_L08_003D3E00);

INCLUDE_ASM("asm/levels", func_L08_003D3EF8);

INCLUDE_ASM("asm/levels", func_L08_003D7E68);

INCLUDE_ASM("asm/levels", func_L08_003D7FB8);

INCLUDE_ASM("asm/levels", func_L08_003D8290);

INCLUDE_ASM("asm/levels", func_L08_003D82F0);
