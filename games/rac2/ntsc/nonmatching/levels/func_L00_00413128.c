#include "include_asm.h"

extern void func_00310440(char *);
extern void func_L00_00414018(void);

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

int func_L00_00413128(char *object)
{
    char *state = *(char **)(object + 0x68);
    char *child;
    char local[0x10];
    int code;

    if (func_00282978(state + 0x1E) != 0) {
        func_00310440(object);
        return -1;
    }

    func_00282D30(local, D_0018C190, 0.75f);
    func_00282CC0(object + 0x10, D_L00_00189EA0 + 0x80, local);
    child = *(char **)(state + 0x20);
    if (child != 0) {
        code = *(unsigned char *)(child + 0x20);
        if (code == 0xFE || code == 0xFD)
            *(char **)(state + 0x20) = 0;
        else if (code == 0)
            func_002AB1F0(child);
    }

    if (*(unsigned char *)(object + 0x20) == 1) {
        func_L00_00414018();
        func_L00_00413358(object);
    }

    return -1;
}
