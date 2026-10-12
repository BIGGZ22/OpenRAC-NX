#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

extern void func_0033AE18(unsigned char *owner, int value);

extern int func_00288FB8_3(int);

extern void func_0033B050(char *, int, int, int);

extern void func_0033B0A8(char *, float);

extern void *func_00288FB8_5(int);

extern void func_0033B050_1(void *, void *, void *, void *);

extern void func_0033B0A8_1(void *, float);

void func_00339760(u8 *a0) {s32 v=1;*(s32 *)(a0+0x300)=v;}

void func_00339770(unsigned char *owner, int value)
{
    func_0033AE18(owner + 8, value);
}

/* Measured target: byte-identical across all 27 levels. */
void func_00339790(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x2f8) = a1; }

/* Calls a one-argument helper three times with three fixed small integers, passes the three results and a pointer plus 8 to a four-argument helper, calls a two-argument helper with the same address and 0.0f, then clears the word at offset 764. */
void func_00339798(char *p)
{
    int first = func_00288FB8_3(11678);
    int second = func_00288FB8_3(11676);

    func_0033B050(p + 8, first, second, func_00288FB8_3(11599));
    func_0033B0A8(p + 8, 0.0f);
    *(int *)(p + 764) = 0;
}

/* looks up three objects by numeric id, runs a four-argument routine and a (pointer,float) routine on a sub-object at self+8, then stores 1 at self+764 */
void func_00339818(int *self)
{
    void *a;
    void *b;
    void *c;
    void *sub = (void *)(self + 2);

    a = func_00288FB8_5(11677);
    b = func_00288FB8_5(11676);
    c = func_00288FB8_5(11599);

    func_0033B050_1(sub, a, b, c);
    func_0033B0A8_1(sub, 0.0f);

    self[191] = 1;
}

INCLUDE_ASM("asm/game", func_003398A0);
