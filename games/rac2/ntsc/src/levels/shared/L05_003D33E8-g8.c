#include "include_asm.h"

extern int D_L05_001B0F50 __attribute__((sda));

void func_L05_00417730(char *object)
{
    char *state = *(char **)(object + 0x68);

    D_L05_001B0F50 = 0;
    *(int *)(state + 0x1BC) = 0;
    *(int *)(state + 0x1B8) = 0;
}

extern void func_00282C88(void *target);

extern void func_00283098(void *a, void *b, void *c);

extern void func_00282CC0(void *a, void *b, void *c);

extern int func_L00_002FCF18(void *p, float, float, float, float, float);

extern int D_L05_001B0FA4 __attribute__((sda));

void func_L05_00418200(char *object)
{
    float local[4];
    float one = 1.0f;

    func_00282C88(local);
    local[0] = 4.0f;
    local[2] = one;
    func_00283098(local, local, object + 0xC0);
    func_00282CC0(local, local, object + 0x10);
    D_L05_001B0FA4 = func_L00_002FCF18(local, 0.0f, 0.0f, 0.0f, one, 0.0f);
}

/* func_L05_0041C118 is catalogued with the native file's marker: its body needs
   the -G8 sibling if it is ever resumed (it reaches D_L05_001B1300 through $gp). */

typedef int TI128 __attribute__((mode(TI)));

extern int func_328508(int);

extern void func_3065F8(int);

extern float func_332C18(void);

extern int D_L05_001D1348 __attribute__((sda));

extern int D_L05_001B14FC __attribute__((sda));

extern int D_L05_001B1504 __attribute__((sda));

extern int D_L05_001B2528 __attribute__((nosda));

extern void func_41D728(int);

extern void func_426270(int);

extern void func_426D98(int);

INCLUDE_ASM("asm/levels", func_L05_0041D498);

INCLUDE_ASM("asm/levels", func_L05_00425FE0);

INCLUDE_ASM("asm/levels", func_L05_00426B08);

extern int D_001A79F0;

extern float func_00282E48(void *, void *);

extern float func_002AA458(void *, void *, void *, float, float, float);

extern float func_00283B98(float, float);

extern void func_L05_0037AB88(int, int);

extern void func_0029CC50(int, int, int);

extern int func_002E5DA0(int, int, char *, int);

extern void func_0029F338(void *);

extern void func_L05_00426348(char *);

void func_L05_004260B8(char *object)
{
    char *state = *(char **)(object + 0x68);
    float scale;

    if (D_L05_001B14FC != 0) {
        scale = func_00282E48((void *)(D_L05_001B14FC + 0x10), object + 0x10);

        if (D_001A79F0 == 0x14) {
            func_002AA458(object + 0x10, (void *)(D_L05_001B14FC + 0x10), state + 0x40,
                         0.008888889104127884f, 0.0033333334140479565f, 1.0666667222976685f);
            goto check_scale;
        }
        if (scale < 60.0f) {
            float *p = *(float **)(object + 0x24);
            float v = p[9] * 0.02222222276031971f * scale;

            *(float *)(object + 0x2C) = v;
            {
                float cap = p[9] * 0.30000001192092896f;

                if (v < cap)
                    *(float *)(object + 0x2C) = cap;
            }
            func_002AA458(object + 0x10, (void *)(D_L05_001B14FC + 0x10), state + 0x40,
                         0.0055555556900799274f, 0.0016666667070239782f, 0.5f);
        }
check_scale:
        if (scale < 0.5f) {
            func_L05_0037AB88(2, 10);
            func_0029CC50(0x1288, 10, -1);
            func_002E5DA0(0xC, 0, object, 0xFBF);
            func_0029F338(object);
            return;
        }
    }
    *(float *)(object + 0xF8) = func_00283B98(*(float *)(object + 0xF8), 0.04f);
    func_L05_00426348(object);
}
