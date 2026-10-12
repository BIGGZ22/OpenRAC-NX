#include "include_asm.h"

extern void Fe524d931_FUN_0033AE28(char *p);

extern void Fe524d931_FUN_0033B018(char *p, float x, float y);

extern void Fe524d931_FUN_002E5E58(int a, int b, int c);

extern void Fe524d931_FUN_00284E40(void);

extern short Fe524d931_D_001A6480[];

extern void Ffb202470_FUN_0033AB50(char *p);

int func_0033BE98(char *object, int mask)
{
    char *sub = object + 8;
    short oldx;
    short oldy;
    int *v;

    Fe524d931_FUN_0033AE28(sub);
    v = *(int **)(object + 684);
    Fe524d931_FUN_0033B018(sub, *(float *)v, *(float *)(v + 1));
    if ((mask & 0x40) != 0 && *(int *)(object + 680) == 0)
        Fe524d931_FUN_002E5E58(4, 0, 0);
    if ((mask & 0xf000) != 0) {
        short *p = Fe524d931_D_001A6480;
        oldx = p[180];
        oldy = p[181];
        if ((mask & 0x1000) != 0) {
            ((unsigned short *)p)[181] = ((unsigned short *)p)[181] - 1;
            if ((short)((unsigned short *)p)[181] < -32)
                p[181] = -32;
        }
        if ((mask & 0x4000) != 0) {
            unsigned short *q = (unsigned short *)Fe524d931_D_001A6480;
            q[181] = q[181] + 1;
            if ((short)q[181] > 32)
                ((short *)q)[181] = 32;
        }
        if ((mask & 0x8000) != 0) {
            unsigned short *q = (unsigned short *)Fe524d931_D_001A6480;
            q[180] = q[180] - 1;
            if ((short)q[180] < -40)
                ((short *)q)[180] = -40;
        }
        if ((mask & 0x2000) != 0) {
            unsigned short *q = (unsigned short *)Fe524d931_D_001A6480;
            q[180] = q[180] + 1;
            if ((short)q[180] > 40)
                ((short *)q)[180] = 40;
        }
        {
            short *q = Fe524d931_D_001A6480;
            if (oldx != q[180] || oldy != q[181])
                Fe524d931_FUN_002E5E58(4, 0, 0);
        }
        Fe524d931_FUN_00284E40();
    }
    return (mask >> 6) & 1;
}

INCLUDE_ASM("asm/game", func_0033C028);

char *func_0033C178(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033C1A8);

INCLUDE_ASM("asm/game", func_0033C1B0);

INCLUDE_ASM("asm/game", func_0033C2D8);

INCLUDE_ASM("asm/game", func_0033C378);

char *func_0033C430(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033C460);
