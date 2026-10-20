#include "include_asm.h"

typedef float f32;
typedef int s32;

extern int func_00283CF0(float);
extern float func_00283CE0(int);

f32 func_L00_0031B9F8(const f32 *a0, const f32 *a1, s32 *a2, f32 factor) {
    s32 index = func_00283CF0(a1[2] / factor);
    f32 first = func_00283CE0(index);
    f32 second = func_00283CE0((index * index + index) >> 1);
    f32 result = a0[2] + a1[2] * first - second * factor;

    if (a2 != 0)
        *a2 = index;
    return result;
}
