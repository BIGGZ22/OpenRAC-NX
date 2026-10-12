#include "include_asm.h"

extern unsigned char *D_L00_001B0CA0 __attribute__((sda));

extern int D_L00_001B2C14 __attribute__((sda));

extern int *D_L00_001B0DB0 __attribute__((sda));

int func_L00_00400F38(char *arg)
{
    unsigned char *p = D_L00_001B0CA0;
    if (!p) {
        return 0;
    }
    return (unsigned int)(p[32] - 5) < 3;
}

int func_L00_00401E28(void)
{
    if (!D_L00_001B0CA0) {
        return 0;
    }
    return D_L00_001B2C14;
}

void func_L00_00402DB8(char *arg)
{
    int *p = D_L00_001B0DB0;
    if (p) {
        int *q = (int *)p[26];
        q[10] = 1;
    }
}

extern void func_L00_00402DB8(char *arg);

void func_L00_004028C0(char *arg, int value)
{
    unsigned char *p = D_L00_001B0CA0;
    int *o;

    if (!p) {
        return;
    }
    o = *(int **)(p + 104);
    p[32] = (arg != 0) ? 5 : 7;
    func_L00_00402DB8(arg);
    if (value != -1) {
        o[24] = value;
    } else {
        o[24] = 10;
    }
}
