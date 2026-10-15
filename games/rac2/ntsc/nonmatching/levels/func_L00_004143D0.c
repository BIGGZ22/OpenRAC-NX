#include "include_asm.h"

typedef struct { unsigned int a, b, c, d; } Vec4u32;
extern char D_L00_001B8DC0[];
extern char *func_003100D8(int);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern void func_002AFFD8(void *, void *, int);
extern void func_00282DD8(void *, void *, void *);

extern char D_L00_0025A920[];
extern void func_L00_00417868(char *, char *);
extern float func_31A7C0(float, float);
extern void func_00281EB8(char *, int, int, int, float, float, float);

extern char D_001A6480[];
extern void func_002FC560(int, int);
extern void func_0027DE50(int, int, int, int, int);
extern void func_L00_00415210(char *, int, int, float, float, float, float, float);

extern char D_L00_0025B98C[];
extern int func_00282948(void *);
extern void func_L00_004173B0(char *);
extern float func_002A7940(void *);
extern void func_002AF000(void *, float, float, float);
extern void func_00282CC0(void *, void *, void *);
extern int func_002761F8(void *, void *, int, void *, int);
extern void func_L00_00417A40(char *, void *, int);
extern void func_L00_00417D70(char *);
extern void func_L00_00417218(char *);
extern void func_L00_00417700(char *);

extern char D_0018C190[];
extern char D_L00_00189EA0[];
extern int func_00282978(void *);
extern void func_00282D30(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_L00_00413358(char *);
extern int func_002AB1F0(void *);

void func_L00_004143D0(char *object)
{
    char *state = *(char **)(object + 0x68);
    int start = *(short *)(state + 0);
    int end = *(short *)(state + 2);
    int delta = end - start;

    if (delta >= 2 && *(short *)(state + 4) != 0 && *(int *)(state + 0x1C) != 0) {
        func_002FC560(0x42, 0x48);
        func_0027DE50(0, *(short *)(D_001A6480 + 0x152), 0,
                      *(short *)(D_001A6480 + 0x150), 0x80808080);
    }

    if (*(int *)(state + 0x18) == 0)
        return;

    if (delta < 10) {
        float current = *(float *)(state + 8);
        float target = *(float *)(state + 0xC);
        float ratio = (float)delta * 0.1f;
        func_L00_00415210(object + 0x10, 0x3C, 0,
                          current + (target - current) * ratio,
                          0.5f, 0.0f, 0.0f, 0.0f);
    } else {
        int kind = *(short *)(state + 0x0);
        int code = *(short *)(state + 0x2);
        if ((unsigned)(kind - 0x2E) < 4) {
            float current = *(float *)(state + 8);
            float target = *(float *)(state + 0xC);
            float ratio = (float)(code - 0x2D) * 0.1f;
            func_L00_00415210(object + 0x10, 0x3C, 0,
                              current + (target - current) * ratio,
                              0.5f, 0.0f, 0.0f, 0.0f);
        }
    }
}
