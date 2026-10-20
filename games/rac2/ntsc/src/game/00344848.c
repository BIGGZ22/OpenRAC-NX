#include "include_asm.h"

extern void func_00335E38(char *p, float x, float y, float zero, float zero2);

extern float D_L04_001AE540 __attribute__((sda));

extern float D_L04_001AE548 __attribute__((sda));

extern float D_L04_001AE550 __attribute__((sda));

extern float D_L04_001AE558 __attribute__((sda));

extern float D_L04_001AE544;

extern float D_L04_001AE54C;

extern float D_L04_001AE554;

extern float D_L04_001AE55C;

struct pos54dfe946 {
    float x;
    float y;
};

void func_00344848(char *self)
{
    float zero = 0.0f;

    func_00335E38(self + 808, D_L04_001AE540 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE544 + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
    func_00335E38(self + 880, D_L04_001AE548 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE54C + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
    func_00335E38(self + 952, D_L04_001AE550 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE554 + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
    func_00335E38(self + 736, D_L04_001AE558 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE55C + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
}

/* These four fill func_00337210's 23-word descriptor, defaults first. f[21] defaults to 1
   before it takes the mode test, as in func_003462A8, whose object still holds that default
   store; here the store is dead and goes, and the later f[22] = 1 reuses its register. */
extern void func_00337210(int *);

extern char D_0025E328[];
extern int D_001AE304 __attribute__((sda));
extern int D_001AE308 __attribute__((sda));
extern int D_001AE310 __attribute__((sda));
extern int D_001AE318 __attribute__((sda));
extern int D_001AE320 __attribute__((sda));
extern int D_001AE328 __attribute__((sda));
extern int D_001AE330 __attribute__((sda));

void func_00344F98(char *o)
{
    int f[23];

    f[16] = 0;
    f[14] = 0;
    f[11] = 0;
    f[13] = 0;
    f[20] = 0;

    f[21] = 1;
    f[19] = *(int *)(o + 0x444);
    f[21] = *(int *)(o + 0x448) == 2;
    f[3] = *(int *)(o + 0x43C);
    f[4] = *(int *)(o + 0x440);
    f[9] = (int)&D_001AE330;
    f[1] = D_001AE304;
    f[2] = D_001AE308;
    f[0] = (int)D_0025E328;
    f[17] = (int)(o + 0x210);
    f[10] = (int)(o + 0x130);
    f[12] = (int)(o + 0x17C);
    f[15] = (int)(o + 0x258);
    f[18] = *(int *)(o + 0x430);
    f[8] = (int)&D_001AE328;
    f[7] = (int)&D_001AE320;
    f[6] = (int)&D_001AE318;
    f[5] = (int)&D_001AE310;
    f[22] = 1;
    func_00337210(f);
}

extern char D_001AA830[];
extern int D_001AE338 __attribute__((sda));
extern int D_001AE33C __attribute__((sda));
extern int D_001AE340 __attribute__((sda));
extern int D_001AE348 __attribute__((sda));
extern int D_001AE350 __attribute__((sda));
extern int D_001AE358 __attribute__((sda));
extern int D_001AE360 __attribute__((sda));

void func_00345088(char *o)
{
    int f[23];

    f[16] = 0;
    f[14] = 0;
    f[11] = 0;
    f[13] = 0;
    f[20] = 0;

    f[21] = 1;
    f[19] = *(int *)(o + 0x444);
    f[21] = *(int *)(o + 0x448) == 1;
    f[3] = *(int *)(o + 0x43C);
    f[4] = *(int *)(o + 0x440);
    f[9] = (int)&D_001AE360;
    f[1] = D_001AE338;
    f[2] = D_001AE33C;
    f[0] = (int)D_001AA830;
    f[17] = (int)(o + 0x210);
    f[10] = (int)(o + 0x130);
    f[12] = (int)(o + 0x17C);
    f[15] = (int)(o + 0x258);
    f[18] = *(int *)(o + 0x430);
    f[8] = (int)&D_001AE358;
    f[7] = (int)&D_001AE350;
    f[6] = (int)&D_001AE348;
    f[5] = (int)&D_001AE340;
    f[22] = 1;
    func_00337210(f);
}

extern char D_001AA870[];
extern int D_001AE368 __attribute__((sda));
extern int D_001AE36C __attribute__((sda));
extern int D_001AE370 __attribute__((sda));
extern int D_001AE378 __attribute__((sda));
extern int D_001AE380 __attribute__((sda));
extern int D_001AE388 __attribute__((sda));
extern int D_001AE390 __attribute__((sda));

void func_00345178(char *o)
{
    int f[23];

    f[16] = 0;
    f[14] = 0;
    f[11] = 0;
    f[13] = 0;
    f[20] = 0;

    f[21] = 1;
    f[19] = *(int *)(o + 0x444);
    f[21] = *(int *)(o + 0x448) == 3;
    f[3] = *(int *)(o + 0x43C);
    f[4] = *(int *)(o + 0x440);
    f[9] = (int)&D_001AE390;
    f[1] = D_001AE368;
    f[2] = D_001AE36C;
    f[0] = (int)D_001AA870;
    f[17] = (int)(o + 0x210);
    f[10] = (int)(o + 0x130);
    f[12] = (int)(o + 0x17C);
    f[15] = (int)(o + 0x258);
    f[18] = *(int *)(o + 0x430);
    f[8] = (int)&D_001AE388;
    f[7] = (int)&D_001AE380;
    f[6] = (int)&D_001AE378;
    f[5] = (int)&D_001AE370;
    f[22] = 1;
    func_00337210(f);
}

extern char D_001AA810[];
extern int D_001AE398 __attribute__((sda));
extern int D_001AE39C __attribute__((sda));
extern int D_001AE3A0 __attribute__((sda));
extern int D_001AE3A8 __attribute__((sda));
extern int D_001AE3B0 __attribute__((sda));
extern int D_001AE3B8 __attribute__((sda));
extern int D_001AE3C0 __attribute__((sda));

void func_00345268(char *o)
{
    int f[23];

    f[16] = 0;
    f[14] = 0;
    f[11] = 0;
    f[13] = 0;
    f[20] = 0;

    f[21] = 1;
    f[19] = *(int *)(o + 0x444);
    f[21] = *(int *)(o + 0x448) == 0;
    f[3] = *(int *)(o + 0x43C);
    f[4] = *(int *)(o + 0x440);
    f[9] = (int)&D_001AE3C0;
    f[1] = D_001AE398;
    f[2] = D_001AE39C;
    f[0] = (int)D_001AA810;
    f[17] = (int)(o + 0x210);
    f[10] = (int)(o + 0x130);
    f[12] = (int)(o + 0x17C);
    f[15] = (int)(o + 0x258);
    f[18] = *(int *)(o + 0x430);
    f[8] = (int)&D_001AE3B8;
    f[7] = (int)&D_001AE3B0;
    f[6] = (int)&D_001AE3A8;
    f[5] = (int)&D_001AE3A0;
    f[22] = 1;
    func_00337210(f);
}
