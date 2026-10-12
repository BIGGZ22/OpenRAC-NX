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

INCLUDE_ASM("asm/game", func_00321E28);

INCLUDE_ASM("asm/game", func_00321E80);

INCLUDE_ASM("asm/game", func_00321F60);

INCLUDE_ASM("asm/game", func_00321FE0);

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

INCLUDE_ASM("asm/game", func_003225C0);

INCLUDE_ASM("asm/game", func_00322680);

INCLUDE_ASM("asm/game", func_00322690);

INCLUDE_ASM("asm/game", func_00322A30);

INCLUDE_ASM("asm/game", func_00322C10);

INCLUDE_ASM("asm/game", func_00322CC0);

INCLUDE_ASM("asm/game", func_003248A8);

INCLUDE_ASM("asm/game", func_003250F8);

INCLUDE_ASM("asm/game", func_00325158);

INCLUDE_ASM("asm/game", func_00325320);

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

INCLUDE_ASM("asm/game", func_00325B28);

INCLUDE_ASM("asm/game", func_00325B70);

INCLUDE_ASM("asm/game", func_00325C50);

INCLUDE_ASM("asm/game", func_00325D30);

INCLUDE_ASM("asm/game", func_00325DA0);

INCLUDE_ASM("asm/game", func_00325EC0);

INCLUDE_ASM("asm/game", func_00326198);

INCLUDE_ASM("asm/game", func_00326A10);

INCLUDE_ASM("asm/game", func_00326BD0);

INCLUDE_ASM("asm/game", func_00326BD8);

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

INCLUDE_ASM("asm/game", func_00326D90);

INCLUDE_ASM("asm/game", func_00326EC0);
