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

void func_L00_00417700(char *owner)
{
    char *base = D_L00_0025A920;
    char *entry = base + 0x30;
    int i;
    float value;

    for (i = 0; i < 24; i++, entry += 0xB0, base += 0xB0) {
        if (*(char **)(entry + 0x6C) != owner)
            continue;
        func_L00_00417868(owner, base);
        if (*(int *)(entry + 0x70) != 0)
            continue;

        value = func_31A7C0(0.125f, 0.25f) * 5.0f;
        func_00281EB8(entry, 0x31, 0x3F7F, 1, value, 1.25f, 0.0f);
        value = func_31A7C0(0.03125f, 0.125f) * 5.0f;
        func_00281EB8(entry, 0x31, 0x5F7F7F7F, 1, value, 1.24f, 0.0f);
        value = func_31A7C0(0.15f, 0.35f) * 5.0f;
        func_00281EB8(entry, 0x0B, 0x103F0000, 1, value, 1.23f, 0.0f);
        *(float *)(entry + 0xCC) = 0.0f;
        *(float *)(entry + 0xDC) = 0.0f;
        *(float *)(entry + 0xEC) = 0.0f;
    }
}
