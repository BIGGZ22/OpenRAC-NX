#include "common.h"

extern f32 func_00283CE0();
extern int func_002EF250(f32);

int func_L00_00329FF0(s32 value) {
    s32 divisor = 16;

    if (value % divisor == 0)
        return 0;
    return (func_002EF250(func_00283CE0() / (f32)divisor) + 1) << 4;
}
