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

INCLUDE_ASM("asm/core", func_00123298);

INCLUDE_ASM("asm/core", func_001232EC);
