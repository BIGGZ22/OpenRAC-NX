#include "include_asm.h"

/* func_002EE6A8: owner ABI/declaration review retained. */
extern int func_002ED688(unsigned char *owner);

#ifndef RAC2_T_V4_FB6552688
#define RAC2_T_V4_FB6552688
typedef int V4_Fb6552688 __attribute__((mode(TI)));
#endif

extern char *func_002ED688_1(char *);

extern void func_00282F20_10(V4_Fb6552688 *, char *, float);

extern void func_00282CC0_26(char *, V4_Fb6552688 *, char *);

extern void func_002EE988(char *);

void func_002EE678(unsigned char *owner, unsigned int value)
{
    unsigned int address = (unsigned int)func_002ED688(owner);
    if (address != 0)
        *(unsigned int *)(address + 0x100u) = value;
}

unsigned int func_002EE6A8(unsigned char *owner)
{
    unsigned int address = (unsigned int)func_002ED688(owner);
    if (address == 0)
        return 0;
    return *(unsigned int *)(address + 0x100u);
}

INCLUDE_ASM("asm/game", func_002EE6D0);

extern int func_002ACCA8(unsigned char *a, int b);
extern void func_002ED9C0(unsigned char *owner);
extern void func_002EE8B0(char *a0);
extern int D_00214F90[];

void func_002EE6E0(unsigned char *owner)
{
    int i;
    unsigned int state;

    if (D_00214F90[36] == 0)
        return;
    if (func_002ED688(owner) == 0)
        return;
    state = func_002EE6A8(owner);
    if (state == 0xB)
        return;
    if (state == 0xD)
        func_002EE678(owner, 1);
    for (i = 0; i < D_00214F90[36]; i++) {
        if (func_002ACCA8(owner + 0x10, D_00214F90[39 + i])) {
            func_002ED9C0(owner);
            func_002EE678(owner, 0xD);
            func_002EE8B0((char *)owner);
            break;
        }
    }
}

extern float func_002A7940(void);
extern float func_00283180(float a0);
extern float func_00283198(float a0);
extern float func_002A7878(float a0, float a1);
extern char *D_001B1C54 __attribute__((sda));

void func_002EE7B8(unsigned char *owner)
{
    char *object;
    char *table;
    float a;
    float b;

    object = (char *)func_002ED688(owner);
    if (object == 0)
        return;
    table = D_001B1C54 + D_00214F90[0] * 128;
    a = func_00283180(func_002A7940()) * func_002A7878(0.0f, *(float *)table);
    *(float *)(object + 0x60) = a;
    b = func_00283198(func_002A7940()) * func_002A7878(0.0f, *(float *)table);
    *(float *)(object + 0x68) = 0.0f;
    *(float *)(object + 0x64) = b;
    func_00282CC0_26(object + 0x60, (V4_Fb6552688 *)(table + 0x30), object + 0x60);
    *(float *)(object + 0x68) = func_002A7878(-64.0f, 64.0f) + *(float *)((char *)owner + 0x18);
    func_002EE988((char *)owner);
    *(int *)(object + 0x130) = 300;
}

/* Calls a callee with the object, returns when it fails, otherwise fills a 16-byte workspace from the object at +192 with 8.0f, hands it to a second callee through +96, calls a third, then stores 300 at +304. */
void func_002EE8B0(char *a0)
{
    V4_Fb6552688 buf;
    char *p;

    p = func_002ED688_1(a0);
    if (p == 0)
        return;
    func_00282F20_10(&buf, a0 + 192, 8.0f);
    func_00282CC0_26(p + 96, &buf, a0 + 16);
    func_002EE988(a0);
    *(int *)(p + 304) = 300;
}

INCLUDE_ASM("asm/game", func_002EE920);

INCLUDE_ASM("asm/game", func_002EE988);
