#include "include_asm.h"

extern float func_00283CE0(int value);
extern int func_00283CF0(float value);

int func_L00_0041A490(int value)
{
    int divisor = 16;
    int scaled;

    if (value % divisor == 0)
        return value;

    scaled = func_00283CF0(func_00283CE0(value) / (float)divisor) + 1;
    return scaled * 16;
}
