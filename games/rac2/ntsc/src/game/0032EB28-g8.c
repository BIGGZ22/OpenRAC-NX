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

extern int D_001B1688;
extern char *D_001B1B68;
extern float ref_L00_00330BC0_00 __attribute__((sda));
extern float func_00282E48(void *, void *);
extern void func_00302440(void *, void *);

void func_00330BC0(char *object)
{
    char *cursor;
    if (D_001B1688 & 1) {
        cursor = D_001B1B68;
        while (cursor) {
            int kind = *(unsigned char *)(cursor + 0x20);
            if (kind == 0xfe) return;
            if (kind == 0xfd) return;
            if (cursor && *(char **)(cursor + 0x24) &&
                *(short *)(*(char **)(cursor + 0x24) + 0x46) == 0x13) {
                if (func_00282E48(cursor + 0x10, object + 0x10) < ref_L00_00330BC0_00)
                    func_00302440(cursor, object);
            }
            cursor = *(char **)(cursor + 0x28);
        }
    }
}

extern float D_001B284C;
/* The retail reaches 0x1B284C twice: through lui/lwc1 (D_001B284C) and through
   $gp+0x385C (ref_L00_0032FEA8_00).  The compiler picks one form per symbol. */
extern float ref_L00_0032FEA8_00 __attribute__((sda));
extern int D_001B2850;
extern float D_001B2854;
extern float D_001B09A0 __attribute__((sda));
extern float func_00282C48(float);
extern void func_00282F20(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_0032FFD8(void *);
extern void func_00330150(void *);
extern void func_003304D8(void *);

void func_0032FEA8(char *object)
{
    char *data = *(char **)(object + 0x68);
    float value;
    int flag;
    char buffer[16];

    if (D_001B284C != 0.0f && D_001B2850 != 0) {
        value = *(float *)(data + 0x58) + D_001B284C * (1.0f / 120.0f);
        *(float *)(data + 0x58) = value;
        if (func_00282C48(value) > D_001B09A0 * (1.0f / 60.0f))
            *(float *)(data + 0x58) = D_001B09A0 * ref_L00_0032FEA8_00 * (1.0f / 60.0f);
    } else {
        *(float *)(data + 0x58) = *(float *)(data + 0x58) * 0.85f;
    }

    flag = D_001B2850;
    value = *(float *)(data + 0x58) * 25.0f;
    *(float *)(object + 0x48) = value;
    if (flag == 0)
        *(float *)(object + 0x48) = value + (D_001B2854 + D_001B2854);

    func_00282F20(buffer, object + 0xC0, *(float *)(data + 0x58));
    func_00282CC0(object + 0x10, object + 0x10, buffer);
    func_0032FFD8(object);
    func_00330150(object);
    func_003304D8(object);
}
