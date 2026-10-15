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



INCLUDE_ASM("asm/game", func_0033C2D8);

typedef struct { int ids[2]; } SolTextPair;
extern SolTextPair D_L00_001AE110;
extern char D_L00_001AE118[];
/* Resident byte at 0x001A7BB1 is shared by all programs. */
extern void func_0033AF48(void *);
extern char *func_00288FB8(int);
extern int func_00115DA8(char *, const char *, ...);
extern void func_0027FAA8(int, int, unsigned long, char *, int);

void func_0033C378(char *object)
{
    char buffer[128];
    SolTextPair pair;
    char *first;
    char *second;
    char *format;
    func_0033AF48(object + 8);
    pair = D_L00_001AE110;
    format = D_L00_001AE118;
    first = func_00288FB8(0x31E4);
    second = func_00288FB8(pair.ids[*(unsigned char *)0x001A7BB1 != 0]);
    func_00115DA8(buffer, format, first, second);
    func_0027FAA8(0x100, 0xAA, 0x80F0F0F0UL, buffer, -1);
}

char *func_0033C430(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033C460);
