#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

extern float func_00282E80_1(char *, float *);

extern float func_00282C48_6(float);

s32 func_002B3868(u8 *a0) {
 u8 flags=a0[0xbe];
 if(((flags^1)&1)!=0) {a0[0xbe]=flags|1;return 1;}
 return 0;
}

/* Sets the second pending flag and reports whether it was newly set. */
s32 func_002B3890(u8 *m) {
    u8 flags = m[0xBE];
    if ((flags & 2) == 0) {
        m[0xBE] = flags | 2;
        return 1;
    }
    return 0;
}

s32 func_002B38B8(u8 *m) {
    return m[0xBE] & 4;
}

extern float func_00282E80(char *, float *);
extern float func_00282E48(char *, char *);
extern float func_00283248(float, float);
extern float func_00283C80(float, float);
extern int D_001A8CA0 __attribute__((sda));
extern int D_0018C0B0;

float func_002B38C8(char *object, char *target)
{
    char *position = object + 0x10;
    char *destination = target + 0x10;
    float distance;
    float value = func_00282E80(position, (float *)destination);
    if (D_001A8CA0 != 0)
        value = func_00282E48(position, destination);
    distance = value;
    if (D_001A8CA0 == 0) {
        float angle = func_00283248(*(float *)(target + 0x10) - *(float *)(object + 0x10),
                                  *(float *)(target + 0x14) - *(float *)(object + 0x14));
        angle = func_00283C80(angle, *(float *)(object + 0xF8));
        distance = distance + angle * distance;
    }
    if (target == (char *)D_0018C0B0)
        distance += 42.0f;
    return distance;
}

/* func_002B39A0 is matched in 002B39A0-g8.c. */

/* Calls a callee on (second pointer, first pointer + 16); if the result is not below the first float argument it returns 0, otherwise it evaluates a second callee on (a float at second+8 minus a float at first+24) and returns 1 when that is below the second float argument, else 0. */
int func_002B3B20(char *p, char *q, float f12, float f13)
{
    float v = func_00282E80_1(q, (float *)(p + 16));

    if (!(v < f12))
        return 0;
    if (func_00282C48_6(*(float *)(q + 8) - *(float *)(p + 24)) < f13)
        return 1;
    return 0;
}

INCLUDE_ASM("asm/game", func_002B3BA8);

INCLUDE_ASM("asm/game", func_002B4028);

typedef struct {
    int reserved;
    int value;
    char remainder[0xA428];
} Record_2B4038;
extern Record_2B4038 D_00240450[];

int func_002B4038(int value, int index) {
    return D_00240450[index].value == value;
}

INCLUDE_ASM("asm/game", func_002B4068);

typedef int PoseVector128 __attribute__((mode(TI)));
typedef struct {
    PoseVector128 initial, previous, current, delta;
    PoseVector128 matrix[4];
    float angles[4];
    PoseVector128 change[4];
} PoseWorkspace;
extern void func_00283410(void *, void *);
extern void func_002830C0(void *, void *, void *);
extern void func_00283788(void *, void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282CC0(void *, void *, void *);
extern void func_002AB6D8(float *, float *);

void func_002B4090(char *self, PoseVector128 *point, float x, float y, float z)
{
    PoseWorkspace work;
    work.initial = *point;
    func_00283410(work.matrix, self + 0xF0);
    func_002830C0(&work.previous, &work.initial, work.matrix);
    work.angles[0] = x;
    work.angles[1] = y;
    work.angles[2] = z;
    func_00283410(work.change, work.angles);
    func_00283788(work.matrix, work.matrix, work.change);
    func_002830C0(&work.current, &work.initial, work.matrix);
    func_00282CF0(&work.delta, &work.previous, &work.current);
    func_00282CC0(self + 0x10, self + 0x10, &work.delta);
    func_002AB6D8((float *)work.matrix, (float *)(self + 0xF0));
}

INCLUDE_ASM("asm/game", func_002B41A0);

INCLUDE_ASM("asm/game", func_002B41A8);

INCLUDE_ASM("asm/game", func_002B44C0);

INCLUDE_ASM("asm/game", func_002B49A0);

extern int D_001AA0F0 __attribute__((sda));

void func_002B49B0(int value) {
    D_001AA0F0 = value;
}

extern int D_001AA0F4 __attribute__((sda));

void func_002B49C0(int value) {
    D_001AA0F4 = value;
}

extern int D_001A8BB8 __attribute__((sda));

int func_002B49C8(int value) {
    return D_001A8BB8 == value;
}

/* func_002B49E0 is matched in 002B49E0-g8.c. */

/* func_002B4B70 is matched in 002B4B70-g8.c. */



INCLUDE_ASM("asm/game", func_002B4C18);

extern char *func_002B5D28(void *);
extern float func_00283B98(float, float);
extern int func_00282978(short *);
extern int func_002A7820(int, int);
extern int func_002B50E0(void *, void *, float);

int func_002B4FF0(char *self, float *target, float amount)
{
    char *data = func_002B5D28(self);
    float angle;
    if (data == 0)
        return 8;
    angle = func_00283248(target[0] - *(float *)(self + 0x10),
                         target[1] - *(float *)(self + 0x14));
    angle = func_00283B98(angle, amount);
    if (*(float *)(data + 0xDC) != 0.0f) {
        angle = func_00283B98(angle, *(float *)(data + 0xDC));
        if (func_00282978((short *)(data + 0xE0))) {
            *(float *)(data + 0xDC) = -*(float *)(data + 0xDC);
            *(short *)(data + 0xE0) = func_002A7820(*(short *)(data + 0xE2), *(short *)(data + 0xE4));
        }
    }
    return func_002B50E0(self, target, angle);
}

INCLUDE_ASM("asm/game", func_002B50D8);

INCLUDE_ASM("asm/game", func_002B50E0);

extern int func_002B50E0(void *, void *, float);

void func_002B5258(char *m) {
    func_002B50E0(m, m + 0x10, *(float *)(m + 0xF8));
}
