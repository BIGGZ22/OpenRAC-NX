#include "include_asm.h"

/* Functions of 0032EB28.c whose retail body reaches its resident object through $gp
   (`addiu $v0,$gp,-0x16C8`). A file named *-g8.c is built with -G8 (config/cflags.txt),
   which is what makes the compiler emit that form. */

extern int D_001AD928 __attribute__((sda));
extern int D_001AD908 __attribute__((sda));
extern void F7242f0a4_FUN_00335AE0(void *);

void func_00335D80(void *object) {
    *(void **)object = &D_001AD928;
    F7242f0a4_FUN_00335AE0(object);
}

void func_00335DA0(void *object) {
    *(void **)object = &D_001AD908;
    F7242f0a4_FUN_00335AE0(object);
}

extern float D_L00_001B2708 __attribute__((sda));
extern void func_00300A00(int, int, float, float, float, float, float);

void func_00336320(char *object)
{
    float *first;
    float *second;
    if (**(float **)(object + 0x10) == 0.0f)
        return;
    if (*(int *)(object + 0x40) == 0)
        return;
    first = *(float **)object;
    second = *(float **)(object + 4);
    func_00300A00(*(int *)(object + 0x40), **(int **)(object + 0xC), first[0], first[1],
                 second[0], second[1] * D_L00_001B2708, **(float **)(object + 0x38));
}

extern void func_002FC808(void);
extern void func_00330EC0(void *);
extern void func_002FDBD8(void *, void *, int, float);
extern float ref_L00_00330E48_00 __attribute__((sda));
void func_00330E48(char *owner)
{
    char *data;
    func_002FC808();
    data = *(char **)(owner + 0x68);
    if (*(float *)(data + 0x64) < ref_L00_00330E48_00)
        func_00330EC0(owner);
    if (*(int *)(data + 0x78))
        func_002FDBD8(data + 0x30, data + 0x40, 0, 0.2f);
}
