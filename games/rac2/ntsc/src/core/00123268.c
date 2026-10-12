#include "include_asm.h"

typedef int s32;

typedef unsigned int u32;

/* Construct a double through the original runtime packing routine. */
typedef struct DoubleParts44 {
    s32 kind;
    u32 sign;
    s32 exponent;
    unsigned long long fraction;
} DoubleParts44;

extern double func_00122630(DoubleParts44 *);

double func_00123268(s32 kind, u32 sign, s32 exponent, unsigned long long fraction)
{
    DoubleParts44 parts;
    parts.kind = kind;
    parts.sign = sign;
    parts.exponent = exponent;
    parts.fraction = fraction;
    return func_00122630(&parts);
}

extern void func_00122760(double *, DoubleParts44 *);
extern float func_001234C0(s32, u32, s32, u32);

float func_00123298(double value) {
    DoubleParts44 parts;
    int fraction;
    func_00122760(&value, &parts);
    fraction = (int)(parts.fraction >> 30);
    if ((parts.fraction & 0x3FFFFFFF) != 0)
        fraction |= 1;
    return func_001234C0(parts.kind, parts.sign, parts.exponent, fraction);
}

INCLUDE_ASM("asm/core", func_001232EC);
