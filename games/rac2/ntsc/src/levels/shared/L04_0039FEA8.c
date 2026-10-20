#include "include_asm.h"

typedef float f32;
typedef unsigned int u32;

extern f32 D_002A7A40[];

/* divides FACTOR by the table's sixteenth entry and scales the whole table by it */
void func_L04_0039FEA8(f32 factor) {
    f32 scale = factor / D_002A7A40[0x10];
    u32 i;

    for (i = 0; i < 0x5C; i++)
        D_002A7A40[i] = D_002A7A40[i] * scale;
}
