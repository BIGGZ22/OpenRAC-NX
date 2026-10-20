#include "include_asm.h"

extern float func_002E5460(int);

extern void func_003109C8(char *, int, int);

void func_L07_003EF1E0(char *arg0, int arg1, int arg2)
{
    int *p = *(int **)(arg0 + 0x68);
    float v;

    p[0] = 0;
    v = func_002E5460(arg1);
    p[2] = arg2;
    *(float *)((char *)p + 4) = 1.0f / v;
    func_003109C8(arg0, 1, -1);
}
