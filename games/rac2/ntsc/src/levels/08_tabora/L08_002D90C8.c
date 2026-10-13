#include "include_asm.h"

typedef unsigned int ti_word __attribute__((mode(TI)));

INCLUDE_ASM("asm/levels", func_L08_002D90C8);

INCLUDE_ASM("asm/levels", func_L08_00308888);

void func_L08_00329EC0(float value, float *object) {
    object[29] = 0.2f;
    object[28] = value - object[6];
}

INCLUDE_ASM("asm/levels", func_L08_0032EA60);

INCLUDE_ASM("asm/levels", func_L08_0032EB48);

INCLUDE_ASM("asm/levels", func_L08_0032EC38);

INCLUDE_ASM("asm/levels", func_L08_0032ED98);

extern int *D_L08_001FE7C0[];
extern float func_00282E80(void *, void *);

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

extern float func_L08_0032EB48(ti_word *, int, float);
extern float func_L08_0032EC38(ti_word *, int, float);
extern float func_L08_0032ED98(ti_word *, int, float);

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


INCLUDE_ASM("asm/levels", func_L08_003CCD28);

INCLUDE_ASM("asm/levels", func_L08_003CCD80);

INCLUDE_ASM("asm/levels", func_L08_003CFE68);

extern char D_L08_002A0D00[];
extern void func_L04_003A21F8(void *, int);

void func_L08_003CFFE8(void) {
    func_L04_003A21F8(D_L08_002A0D00, 7);
}

extern void func_L04_003A02A0(void *, int);

void func_L08_003D0010(void) {
    func_L04_003A02A0(D_L08_002A0D00, 7);
}

extern void func_0029FCC0(void *, int, void *);

void func_L08_003D0040(void *object) {
    void **parts = *(void ***)((char *)object + 0x68);
    if (parts[0]) func_0029FCC0(object, 0, (char *)parts[0] + 0x10);
    if (parts[1]) func_0029FCC0(object, 1, (char *)parts[1] + 0x10);
}

INCLUDE_ASM("asm/levels", func_L08_003D0098);

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
