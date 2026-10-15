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

int func_L00_00416F68(char *owner, char *arg1, int arg2)
{
    char *object = func_003100D8(0x139E);
    char *state;
    char *first;
    char *second;
    Vec4u32 fill = { 0xFF, 0, 0, 0 };
    char local[0x10];

    if (object == 0)
        return 0xFF;

    state = *(char **)(object + 0x68);
    first = object + 0x10;
    *(unsigned char *)(object + 0x30) = 0xFF;
    *(unsigned short *)(object + 0x32) = 0xFFFF;
    *(unsigned char *)(object + 0x20) = 0;
    *(Vec4u32 *)owner = (Vec4u32){ 0xFF, 0, 0, 0 };
    *(Vec4u32 *)first = (Vec4u32){ 0xFF, 0, 0, 0 };
    *(float *)(object + 0x1C) = 1.0f;
    second = object + 0xC0;

    func_00282CF0(local, D_L00_001B8DC0, first);
    func_00282F20(local, local, 1.0f);
    func_002AFFD8(first, local + 0x10, 1);
    func_00282DD8(object + 0xD0, second, local + 0x10);
    func_00282F20(object + 0xD0, object + 0xD0, 1.0f);
    func_00282DD8(object + 0xE0, object + 0xD0, second);

    *(float *)(object + 0xCC) = 0.0f;
    *(float *)(object + 0xEC) = 0.0f;
    *(float *)(object + 0xDC) = 0.0f;
    *(int *)(state + 8) = arg2;
    *(int *)(state + 0) = 0x3C;
    *(char **)(state + 4) = arg1;
    return (int)object;
}
