#include "include_asm.h"

extern char D_00189E20_9[];

extern float func_00282C88_1(float *buf);

extern float func_00282DF0(float *buf);

INCLUDE_ASM("asm/game", func_0031F210);

INCLUDE_ASM("asm/game", func_0031F6E8);

INCLUDE_ASM("asm/game", func_0031FC28);

INCLUDE_ASM("asm/game", func_003202E8);

INCLUDE_ASM("asm/game", func_00320550);

INCLUDE_ASM("asm/game", func_003216E8);

extern void func_0029F338(void *);
extern void func_00321F60(char *);

void func_00321E28(char *object)
{
    char *data = *(char **)(object + 0x68);
    char *child = *(char **)(data + 0x2AC);
    if (child != 0 && *(short *)(child + 0xAA) == 0xAB7) {
        func_0029F338(child);
        *(char **)(data + 0x2AC) = 0;
    }
    func_00321F60(object);
}

extern void func_002AB8C0(char *, char *);
extern int func_00282978(void *);
extern void func_002AEC48(void *, void *, int, float, float);
extern void func_002B0990(char *, char *, int, int, float);

void func_00321E80(char *object)
{
    char *data = *(char **)(object + 0x68);
    char *first;
    func_002AB8C0(object, data + 0x50);
    first = data + 0x3D0;
    func_00282978(data + 0x270);
    func_00282978(data + 0x272);
    func_002AEC48(object, first, 5, 0.03f, 0.3f);
    func_002AEC48(object, data + 0x450, 6, 0.03f, 0.3f);
    if (*(unsigned short *)(object + 0x96) < 65535)
        (*(unsigned short *)(object + 0x96))++;
    func_002B0990(object, first, 5, 1, 2.5f);
}

/* func_00321FE0 is in the -G8 sibling. */

void func_003220A8(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/game", func_00322138);

INCLUDE_ASM("asm/game", func_00322340);


INCLUDE_ASM("asm/game", func_00322680);

INCLUDE_ASM("asm/game", func_00322690);


extern int func_00273B70(char *);
extern void func_002A7750(unsigned char *, unsigned char, int);
void func_00322C10(char *object)
{
    char *data = *(char **)(object + 0x68);
    *(float *)(data + 4) -= 1.0f / 120.0f;
    switch (*(unsigned char *)(object + 0x20)) {
    case 0:
        if (func_00273B70(object)) {
            func_002A7750((unsigned char *)object, 1, -1);
            *(unsigned short *)(object + 0x34) |= 0x2800;
        }
        break;
    case 1:
        func_00273B70(object);
        if (*(unsigned char *)(object + 0x23) < 2)
            func_0029F338(object);
        else
            *(unsigned char *)(object + 0x23) -= 2;
        break;
    }
}

INCLUDE_ASM("asm/game", func_00322CC0);

INCLUDE_ASM("asm/game", func_003248A8);

extern void func_00325D30(char *, char *);
extern void func_00325320(char *);

void func_003250F8(char *object)
{
    char *data = *(char **)(object + 0x68);
    char *child;
    func_00325D30(object, data + 0x2A0);
    child = *(char **)(data + 0x2E0);
    if (child != 0 && *(short *)(child + 0xAA) == 0xAB7) {
        func_0029F338(child);
        *(char **)(data + 0x2E0) = 0;
    }
    func_00325320(object);
}

INCLUDE_ASM("asm/game", func_00325158);

/* func_00325320 is in the -G8 sibling. */

void func_003253A0(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/game", func_00325430);

INCLUDE_ASM("asm/game", func_00325638);

INCLUDE_ASM("asm/game", func_003258B8);

INCLUDE_ASM("asm/game", func_003258C0);

INCLUDE_ASM("asm/game", func_00325A88);

/* func_00325B28 is in the -G8 sibling. */

extern char *func_0029EFD0(int);
extern void func_002B37E8(void *, void *);
extern void func_002AF1D0(void *, void *, int, int);
extern void func_002A0F48(void *);

void func_00325B70(char *self, char **children, int first, int second)
{
    int i;
    for (i = 0; i < 2; i++, children++) {
        *children = func_0029EFD0(0xAA2);
        if (*children != 0) {
            func_002B37E8(*children, self);
            *(float *)(*children + 0x2C) *= 0.75f;
            {
                char *child = *children;
                if (i != 0)
                    child[0xBC] = second;
                else
                    child[0xBC] = first;
            }
            *(unsigned short *)(*children + 0x32) = *(unsigned short *)(self + 0x32);
            func_002AF1D0(self, *children, *(unsigned char *)(*children + 0xBC), 0);
            func_002A0F48(*children);
        }
    }
}

INCLUDE_ASM("asm/game", func_00325C50);

void func_00325D30(char *object, char *entries)
{
    int count = 1;
    do {
        char *child = *(char **)entries;
        count--;
        entries += 4;
        if (child != 0 && *(short *)(child + 0xAA) == 0xAA2)
            func_0029F338(child);
    } while (count >= 0);
}

INCLUDE_ASM("asm/game", func_00325DA0);

INCLUDE_ASM("asm/game", func_00325EC0);

INCLUDE_ASM("asm/game", func_00326198);

INCLUDE_ASM("asm/game", func_00326A10);

INCLUDE_ASM("asm/game", func_00326BD0);



INCLUDE_ASM("asm/game", func_00326CA8);

/* negates two floats read from a fixed global table, clamps each to [-1,1] into an object reached through the parameter's +104 pointer, packs the pair into a two-float stack buffer, passes it to two callees and stores the second callee's float return value at object+48 */
void func_00326CB8(char *obj)
{
    float buf[2];
    char *d = D_00189E20_9;
    char *e = *(char **)(obj + 104);
    float a, b, x;

    a = -*(float *)(d + 8112);
    *(float *)(e + 20) = a;
    b = -*(float *)(d + 8116);
    *(float *)(e + 16) = b;

    if (a > 1.0f) {
        *(float *)(e + 20) = 1.0f;
    } else if (a < -1.0f) {
        *(float *)(e + 20) = -1.0f;
    }

    x = *(float *)(e + 16);
    if (x > 1.0f) {
        *(float *)(e + 16) = 1.0f;
    } else if (x < -1.0f) {
        *(float *)(e + 16) = -1.0f;
    }
    func_00282C88_1(buf);
    buf[0] = *(float *)(e + 16);
    buf[1] = *(float *)(e + 20);
    *(float *)(e + 48) = func_00282DF0(buf);
}



INCLUDE_ASM("asm/game", func_00326EC0);
