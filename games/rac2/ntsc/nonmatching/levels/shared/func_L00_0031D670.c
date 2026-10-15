#include "common.h"

typedef struct {
    f32 x, y, z, w;
} LocalV4;

extern f32 func_002AA140(f32, f32, f32);

void func_L00_0031D670(LocalV4 *out, const LocalV4 *a, const LocalV4 *b, f32 factor) {
    LocalV4 left = *a;
    LocalV4 right = *b;

    out->x = func_002AA140(left.x, right.x, factor);
    out->y = func_002AA140(left.y, right.y, factor);
    out->z = func_002AA140(left.z, right.z, factor);
    out->w = func_002AA140(left.w, right.w, factor);
}
