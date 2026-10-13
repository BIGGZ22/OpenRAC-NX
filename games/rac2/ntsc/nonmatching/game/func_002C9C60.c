#include "include_asm.h"

extern struct {
    char pad[0x1C0];
    float one;
    float reciprocal;
    int mode;
} D_001F2840;

void func_002C9C60(float x)
{
    float q = 1.0f / x;
    D_001F2840.one = 1.0f;
    D_001F2840.reciprocal = q;
    D_001F2840.mode = 0;
}
