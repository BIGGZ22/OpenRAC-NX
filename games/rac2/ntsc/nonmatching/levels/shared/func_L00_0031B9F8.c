#include "common.h"

extern f32 func_00283CE0();
extern s32 func_00283CF0(f32);

f32 func_L00_0031B9F8(const f32 *a0, const f32 *a1, s32 *out, f32 factor) {
    s32 index = func_00283CF0(a1[2] / factor);
    f32 first = func_00283CE0(index);
    f32 second = func_00283CE0(index);
    f32 result = a0[2] + a1[2] * first - second * factor;

    if (out != 0)
        *out = index;
    return result;
}
