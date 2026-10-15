#include "include_asm.h"

extern void func_002E6EF8(char *, char *);
extern void func_00310440(char *);

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

void func_L00_004170A8(char *object)
{
    char *state = *(char **)(object + 0x68);
    char local[0x10];
    float first;
    float second;
    int i;

    if (func_00282948(state) != 0) {
        for (i = 0; i < 24; i++)
            *(int *)(D_L00_0025B98C - i * 0xB0) = 0;
    } else {
        if (*(int *)(state + 4) == 0) {
            func_L00_004173B0(object);
            if (*(int *)(state + 4) == 0) {
                first = func_002A7940(object + 0x10);
                second = func_002A7940(object + 0x10);
                func_002AF000(local, 7.0f, first, second);
            } else {
                first = func_002A7940(object + 0x10);
                second = func_002A7940(object + 0x10);
                func_002AF000(local, 5.0f, first, second);
            }
        }

        func_00282CC0(local, local, object + 0x10);
        if (func_002761F8(object + 0x10, local, 0, object, 0) != 0) {
            *(long long *)((char *)D_L00_0025B98C - 0xDE0) = 0;
            *(long long *)(local + 0x10) = 0;
        }
        func_L00_00417A40(object, local, 0x14);
        func_L00_00417D70(object);
        func_002E6EF8((char *)func_L00_00417700, object);
    }

    if (*(int *)(state + 4) == 0) {
        func_L00_00417218(object);
        func_00310440(object);
    }
}
